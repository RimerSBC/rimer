/**-----------------------------------------------------------------------------
 * Copyright (c) 2025 Sergey Sanders
 * sergey@sesadesign.com
 * -----------------------------------------------------------------------------
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0
 * International (CC BY-NC-SA 4.0).
 *
 * You are free to:
 *  - Share: Copy and redistribute the material.
 *  - Adapt: Remix, transform, and build upon the material.
 *
 * Under the following terms:
 *  - Attribution: Give appropriate credit and indicate changes.
 *  - NonCommercial: Do not use for commercial purposes.
 *  - ShareAlike: Distribute under the same license.
 *
 * DISCLAIMER: This work is provided "as is" without any guarantees. The authors
 * aren’t responsible for any issues, damages, or claims that come up from using
 * it. Use at your own risk!
 *
 * Full license: http://creativecommons.org/licenses/by-nc-sa/4.0/
 * ---------------------------------------------------------------------------*/

/**
 * @file rshell.c
 * @author Sergey Sanders
 * @date February 2024
 * @brief * rshell interface "Serial IO"
 *   system control interface
 *
 */
#include "FreeRTOS.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "keyboard.h"
#include "rshell.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include "i2c_port.h"
#include "iface_dio.h"
#include "enums.h"

#define SIO_SERDES_CLOCK 12000000.0
#define regUARTbaud(x) (uint16_t)(65536.0 * (1.0 - 16.0 * (x / SIO_SERDES_CLOCK)))
#define RX_BUFF_SIZE 32                   // should be a power of 2
const uint8_t I2CspeedDiv[] = {54, 9, 2}; /// Baud rate selections

void set_sio_mode(uint8_t chan, uint8_t mode);
void set_sio_baud(uint8_t chan, uint32_t baud);
static bool iface_sio_init(bool verbose);
static cmd_err_t cmd_sio_channel(_cl_param_t *sParam);
static cmd_err_t cmd_sio_baud(_cl_param_t *sParam);
static cmd_err_t cmd_sio_mode(_cl_param_t *sParam);
static cmd_err_t cmd_sio_tx(_cl_param_t *sParam);
static cmd_err_t cmd_sio_rx(_cl_param_t *sParam);
static cmd_err_t cmd_sio_scan(_cl_param_t *sParam);
static cmd_err_t cmd_sio_power(_cl_param_t *sParam);
static char promptBuff[8];

const _iface_t ifaceSIO =
    {
        .name = "sio",
        .prompt = promptBuff,
        .desc = "Serial IO ports control",
        .init = iface_sio_init,
        .cmdList =
            {
                {.name = "chan", .desc = "set channel 0/1", .func = cmd_sio_channel},
                {.name = "pwr", .desc = "pwr : \'0\'=OFF, \'1\'=ON", .func = cmd_sio_power},
                {.name = "baud", .desc = "set port baud/bit rate", .func = cmd_sio_baud},
                {.name = "mode", .desc = "set mode 0:3(OFF,SPI,UART,I2C)", .func = cmd_sio_mode},
                {.name = "tx", .desc = "send B1 [B2 ... B7]", .func = cmd_sio_tx},
                {.name = "rx", .desc = "receive Size [Mode/Addr/TX]", .func = cmd_sio_rx},
                {.name = "scan", .desc = "Scan I2C bus", .func = cmd_sio_scan},
                {.name = NULL, .func = NULL},
            }};
enum
{
   SIO_MODE_NONE = 0,
   SIO_MODE_SPI,
   SIO_MODE_UART,
   SIO_MODE_I2C,
};

const struct
{
   Sercom *sercom[3];
   PortGroup *pport[3];
   uint8_t irq[3];
   uint32_t pins[3][2];
} sioPort =
    {
        .sercom = {DIO0_SERCOM, DIO1_SERCOM, UART_SERCOM},
        .pport = {&DIO0_PORT, &DIO1_PORT, &UART_PORT},
        .irq = {SERCOM1_2_IRQn, SERCOM2_2_IRQn, SERCOM5_2_IRQn},
        .pins = {
            {
                // channel 0
                DIO0_PIN_PAD0 | DIO0_PIN_PAD1 | DIO0_PIN_PAD2 | DIO0_PIN_PAD3, // SIO: serial port is set to digital; SPI MOSI - PAD[0], SCK - PAD[1], *SS - PAD[2], MISO - PAD[3]
                DIO0_PIN_PAD0 | DIO0_PIN_PAD1,                                 // UART: TX - PAD[0], RX - PAD[1], I2C: SDA - PAD[0], SCL - PAD[1]
            },
            {
                // channel 1
                DIO1_PIN_PAD0 | DIO1_PIN_PAD1 | DIO1_PIN_PAD2 | DIO1_PIN_PAD3, // SIO: serial port is set to digital
                DIO1_PIN_PAD0 | DIO1_PIN_PAD1,
            },
            {
                // channel 2
                UART_PIN_RXD | UART_PIN_TXD, // SIO: serial port is set to digital
                UART_PIN_RXD | UART_PIN_TXD,
            },
        }};

static struct
{
   uint8_t data[RX_BUFF_SIZE];
   uint8_t head;
   uint8_t tail;
} sioFifo[3] = {0};

const char *mName[4] = {"SIO", "SPI", "UART", "I2C"};

static void prompt_update(void)
{
   tsprintf((char *)ifaceSIO.prompt, "%s:%d", mName[ioConf.mode[ioConf.sioChan]], ioConf.sioChan);
}

static bool iface_sio_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;
   REG_GCLK_PCHCTRL8 = CLK_12MHZ | GCLK_PCHCTRL_CHEN;  /* SERCOM1 core clock @ 12MHz (DIO0_SERCOM)*/
   REG_GCLK_PCHCTRL23 = CLK_12MHZ | GCLK_PCHCTRL_CHEN; /* SERCOM2 core clock @ 12MHz (DIO1_SERCOM)*/
   REG_GCLK_PCHCTRL35 = CLK_12MHZ | GCLK_PCHCTRL_CHEN; /* SERCOM5 core clock @ 12MHz (UART_SERCOM)*/
   REG_MCLK_APBAMASK |= MCLK_APBAMASK_SERCOM1;
   REG_MCLK_APBBMASK |= MCLK_APBBMASK_SERCOM2;
   REG_MCLK_APBDMASK |= MCLK_APBDMASK_SERCOM5;
   if (ioConf.checkSum == crc8((uint8_t *)&ioConf,sizeof(ioConf)-1)) // Config is valid, init the ports
   {
       for (uint8_t i=0;i<3;i++)
       if (ioConf.mode[i])
       {   
           set_sio_mode(ioConf.sioChan,ioConf.mode[ioConf.sioChan]);
           set_sio_baud(ioConf.sioChan,ioConf.baud[ioConf.sioChan]);
       }
   }
   prompt_update();
   initialized = true;
   if (verbose)
      tprintf("Interface \"sio\" initialized.\n");
   return true;
}

void set_sio_baud(uint8_t chan, uint32_t baud)
{
   uint8_t i2cSpeed;
   Sercom *sPort = (Sercom *)sioPort.sercom[chan];
   if (!ioConf.mode[chan])
      return;
   ioConf.baud[chan] = baud;
   sPort->USART.CTRLA.bit.ENABLE = 0;
   while (sPort->SPI.SYNCBUSY.bit.ENABLE)
      __asm("nop");
   switch (ioConf.mode[chan])
   {
   case SIO_MODE_NONE: // should not get here
      break;
   case SIO_MODE_SPI:
      if (ioConf.baud[chan] > (SIO_SERDES_CLOCK / 2)) ioConf.baud[chan] = SIO_SERDES_CLOCK / 2;
      sPort->SPI.BAUD.reg = (SIO_SERDES_CLOCK / (2 * ioConf.baud[chan])) - 1;   // calculate clock
      //ioConf.baud[chan] = (SIO_SERDES_CLOCK / (2 * (sPort->SPI.BAUD.reg + 1))); // update the config with closet baudrate
      break;
   case SIO_MODE_UART:
      sPort->USART.BAUD.reg = regUARTbaud(ioConf.baud[chan]);
      break;
   case SIO_MODE_I2C:
      i2cSpeed = (ioConf.baud[chan] == 1000000 ? I2C_SPEED_1MHZ : ioConf.baud[chan] == 400000 ? I2C_SPEED_400KHZ :
                                                                                                  I2C_SPEED_100KHZ);
      if (!i2cSpeed)
         ioConf.baud[chan] = 100000;                    // correct I2C baud setting
      sPort->I2CM.BAUD.bit.BAUD = I2CspeedDiv[i2cSpeed]; /// Set baud rate
      break;
   }
   sPort->USART.CTRLA.bit.ENABLE = 0x01; /// Enable SERCOM: USART will work for all modes
   while (sPort->I2CM.SYNCBUSY.bit.ENABLE)
      __asm("nop"); /// Check to see if SERCOM synchronization is busy
}

void set_sio_mode(uint8_t chan, uint8_t mode)
{
   Sercom *sPort = (Sercom *)sioPort.sercom[chan];
   mode &= 0x03;
   PortGroup *pPort = (PortGroup *)sioPort.pport[chan];
   NVIC_DisableIRQ(sioPort.irq[chan]);
   pPort->WRCONFIG.reg = CONF_PIN_VAL(sioPort.pins[chan][0]) | PORT_WRCONFIG_WRPINCFG; // de-mux the port
   if (mode)
      pPort->WRCONFIG.reg = CONF_PIN_VAL(sioPort.pins[chan][mode > 1 ? 1 : 0]) | PORT_WRCONFIG_PMUX((chan == 2) ? 3 : 2) | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_WRPINCFG; // mux the port
   sPort->SPI.CTRLA.bit.SWRST = 1;                                                                                                                                                               // reset the port
   while (sPort->SPI.SYNCBUSY.bit.SWRST)
      __asm("nop");

   switch (mode)
   {
   case SIO_MODE_NONE:
      break;
   case SIO_MODE_SPI:
      sPort->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE(3) | SERCOM_SPI_CTRLA_DIPO(0x03) | SERCOM_SPI_CTRLA_DOPO(0x00);
      /// Hardware *SS enable
      sPort->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN | SERCOM_SPI_CTRLB_MSSEN;
      set_sio_baud(chan, ioConf.baud[chan]); // set clock and Enable the port
      break;
   case SIO_MODE_UART:
      sPort->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE(1) | SERCOM_USART_CTRLA_TXPO(0) | SERCOM_USART_CTRLA_RXPO(1) | SERCOM_USART_CTRLA_DORD;
      sPort->USART.CTRLB.reg = SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;
      set_sio_baud(chan, ioConf.baud[chan]); // set clock and Enable the port
      sPort->USART.INTENSET.bit.RXC = 1;      // Receive Complete Interrupt Enable
      NVIC_EnableIRQ(sioPort.irq[chan]);
      break;
   case SIO_MODE_I2C:
      sPort->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE(5) | SERCOM_I2CM_CTRLA_SPEED(1) | SERCOM_I2CM_CTRLA_LOWTOUTEN | SERCOM_I2CM_CTRLA_SEXTTOEN | SERCOM_I2CM_CTRLA_MEXTTOEN | SERCOM_I2CM_CTRLA_SCLSM;
      sPort->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN; /// Enable smart mode
      set_sio_baud(chan, ioConf.baud[chan]);         // set clock and Enable the port
      sPort->I2CM.STATUS.bit.BUSSTATE = 0x01;         /// Set bus state to I2C_STAT_IDLE
      break;
   }
   ioConf.mode[chan] = mode;
   prompt_update();
}

static cmd_err_t cmd_sio_channel(_cl_param_t *sParam)
{
   if (sParam->argc)
   {
      ioConf.sioChan = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (ioConf.sioChan > 2)
         ioConf.sioChan = 0;
      set_sio_mode(ioConf.sioChan, ioConf.mode[ioConf.sioChan]);
      prompt_update();
   }
   else
      tprintf("%d\n", ioConf.sioChan);
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_power(_cl_param_t *sParam)
{
   uint8_t chan = ioConf.sioChan;
   switch (sParam->argc)
   {
   case 0:
      tprintf("%s\n", (ioConf.sioChan == 2 ? PWR_UART_PORT.OUT.reg & PWR_UART_EN : PWR_PORT.OUT.reg & (ioConf.sioChan ? PWR_DIO1_EN : PWR_DIO0_EN)) ? "OFF" : "ON");
      break;
   case 1:
      set_io_power(chan, tget_enum(sParam->argv[0], EnumOnOff));
      break;
   case 2:
      chan = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (chan > 2) chan = 2;
      set_io_power(chan, tget_enum(sParam->argv[1], EnumOnOff));
      break;
   }
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_baud(_cl_param_t *sParam)
{
   uint8_t chan = ioConf.sioChan;
   switch (sParam->argc)
   {
   case 0:
      tprintf("%d bps\n", ioConf.baud[chan]);
      break;
   case 1:
      set_sio_baud(chan, (uint32_t)strtol(sParam->argv[0], NULL, 0));
      break;
   case 2:
      chan = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (chan > 2) chan = 2;
      set_sio_baud(chan, (uint32_t)strtol(sParam->argv[1], NULL, 0));
      break;
   }
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_mode(_cl_param_t *sParam)
{
   char *paramModeStr;
   uint8_t chan = ioConf.sioChan;
   switch (sParam->argc)
   {
   case 0:
      tprintf("  Chan: %d\n  Mode: %s\n  Baud: %d\n", chan, mName[ioConf.mode[chan]], ioConf.baud[chan]);
      return CMD_NO_ERR;
   case 1:
      paramModeStr = sParam->argv[0];
      break;
   case 2:
      chan = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (chan > 2) chan = 2;
      paramModeStr = sParam->argv[1];
      break;
    default: return CMD_TOOMANY_PARAM;
   }
   uint8_t mode = tget_enum(paramModeStr, EnumSerPort);
   if ((mode == SIO_MODE_SPI) && (chan == 2))
      return "No SPI on channel 2";
   set_sio_mode(chan, mode);
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_tx(_cl_param_t *sParam)
{
   uint8_t err = 0;
   uint8_t i;
   char *str;
   Sercom *port = (Sercom *)sioPort.sercom[ioConf.sioChan];
   switch (ioConf.mode[ioConf.sioChan])
   {
   case SIO_MODE_UART:
      if (sParam->argc) // send string
      {
         for (uint8_t i = 0; i < sParam->argc; i++)
         {
            str = sParam->argv[i];
            if (*str < '0' || *str > '9') // send string
               while (*str)
               {
                  while (!port->USART.INTFLAG.bit.DRE)
                     __asm("nop");
                  port->USART.DATA.reg = *str++;
               }
            else
            {
               while (!port->USART.INTFLAG.bit.DRE)
                  __asm("nop");
               port->USART.DATA.reg = (uint8_t)strtol(str, NULL, 0);
            }
         }
      }
      else
         for (i = 0; i < sParam->argc; i++)
         {
            while (!port->USART.INTFLAG.bit.DRE)
               __asm("nop");
            port->USART.DATA.reg = (uint8_t)strtol(sParam->argv[i], NULL, 0);
         }
      break;
   case SIO_MODE_SPI:
      for (i = 0; i < sParam->argc; i++)
      {
         while (!port->SPI.INTFLAG.bit.DRE)
            __asm("nop");
         port->SPI.DATA.reg = (uint8_t)strtol(sParam->argv[i], NULL, 0);
         while (!port->SPI.INTFLAG.bit.RXC)
            __asm("nop");
         tprintf("0x%2x ", (uint8_t)port->SPI.DATA.reg);
      }
      tprintf("\n");
      break;
   case SIO_MODE_I2C:
      if (!sParam->argc)
         return "DevID missing.";
      for (i = 0; i < sParam->argc && i < sizeof(sioFifo[ioConf.sioChan].data); i++)
         sioFifo[ioConf.sioChan].data[i] = (uint8_t)strtol(sParam->argv[i], NULL, 0);
      if ((err = i2c_bare_write(port, sioFifo[ioConf.sioChan].data[0] << 1, &sioFifo[ioConf.sioChan].data[1], sParam->argc - 1)) != I2C_ERR_NONE)
      {
         tprintf("E:I2C error[%d]\n", err);
         return CMD_ERR_EMPTY;
      }
      else
         tprintf("%d bytes sent.\n", sParam->argc - 1);
      break;
   default:
      break;
   }
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_rx(_cl_param_t *sParam)
{
   uint8_t size = 1, err;
   uint16_t addr = 0xff, DevID;
   uint8_t i;
   Sercom *port = (Sercom *)sioPort.sercom[ioConf.sioChan];

   switch (ioConf.mode[ioConf.sioChan])
   {
   case SIO_MODE_UART:
      size = 0;
      while (sioFifo[ioConf.sioChan].tail != sioFifo[ioConf.sioChan].head)
      {
         tprintf("%c", sioFifo[ioConf.sioChan].data[sioFifo[ioConf.sioChan].tail++]);
         sioFifo[ioConf.sioChan].tail &= sizeof(sioFifo[ioConf.sioChan].data) - 1; // keep the pointer within the buffer address range
         size++;
      }
      break;
   case SIO_MODE_SPI:
      if (sParam->argc)
         size = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (sParam->argc > 1)
         addr = (uint8_t)strtol(sParam->argv[1], NULL, 0);
      while (size--)
      {
         while (!(sioPort.sercom[ioConf.sioChan]->SPI.INTFLAG.bit.DRE))
            __asm("nop");
         sioPort.sercom[ioConf.sioChan]->SPI.DATA.reg = addr;
         while (!sioPort.sercom[ioConf.sioChan]->SPI.INTFLAG.bit.RXC)
            __asm("nop");
         tprintf("0x%2x ", (uint8_t)sioPort.sercom[ioConf.sioChan]->SPI.DATA.reg);
      }
      break;
   case SIO_MODE_I2C:
      DevID = (sParam->argc) ? (uint8_t)strtol(sParam->argv[0], NULL, 0) << 1 : 0;
      switch (sParam->argc)
      {
      case 0:
         return "DevID missing.";
      case 1:
         return "Size missing.";
      case 2: // bare read - DEV ID + Read Bit only.
         break;
      case 3:
      case 4:
         addr = (uint8_t)strtol(sParam->argv[1], NULL, 0);
         if (sParam->argc == 4)
         {
            DevID += 0x0100; // two bytes address
            addr += (uint8_t)strtol(sParam->argv[2], NULL, 0) << 8;
         }
         break;
      default:
         return CMD_MISSING_PARAM;
      }
      size = (uint8_t)strtol(sParam->argv[sParam->argc - 1], NULL, 0);
      if (size > RX_BUFF_SIZE)
         size = RX_BUFF_SIZE;
      if (sParam->argc == 2)
         err = i2c_bare_read(port, (uint8_t)DevID, sioFifo[ioConf.sioChan].data, size);
      else
         err = i2c_read(port, DevID, addr, sioFifo[ioConf.sioChan].data, size);
      if (err)
      {
         tprintf("E: I2C error[%d]\n", err);
         return CMD_ERR_EMPTY;
      }
      else
         for (i = 0; i < size; i++)
            tprintf("0x%2x ", sioFifo[ioConf.sioChan].data[i]);
      break;
   default:
      return CMD_NO_ERR;
   }
   if (size)
      tprintf("\n");
   return CMD_NO_ERR;
}

static cmd_err_t cmd_sio_scan(_cl_param_t *sParam)
{

   if (ioConf.mode[ioConf.sioChan] != SIO_MODE_I2C)
      return "I2C scan: invalid mode!";
   Sercom *port = (Sercom *)sioPort.sercom[ioConf.sioChan];
   uint8_t tmp;
   for (uint8_t i = 0; i < 128; i++)
   {
      if (i2c_bare_read(port, i << 1, &tmp, 1) == I2C_ERR_NONE)
      {
         tprintf("0x%2x\n", i);
         vTaskDelay(1);
      }
      taskYIELD();
   }
   return CMD_NO_ERR;
}

void SERCOM1_2_Handler(void) // Receive Complete Interrupt
{
   uint8_t head = (sioFifo[0].head + 1) & (sizeof(sioFifo[0].data) - 1);
   if (sioPort.sercom[0]->USART.INTFLAG.bit.RXC)
   {
      if (head != sioFifo[0].tail)
         sioFifo[0].data[sioFifo[0].head++] = sioPort.sercom[0]->USART.DATA.reg;
      else
         head = sioPort.sercom[0]->USART.DATA.reg; // clear the input buffer
   }
   sioFifo[0].head &= sizeof(sioFifo[0].data) - 1;                              // keep the pointer within the buffer address range
   sioPort.sercom[0]->USART.INTFLAG.reg = sioPort.sercom[0]->USART.INTFLAG.reg; // Clear interrupt flags
}

void SERCOM2_2_Handler(void) // Receive Complete Interrupt
{
   uint8_t head = (sioFifo[1].head + 1) & (sizeof(sioFifo[1].data) - 1);
   if (sioPort.sercom[1]->USART.INTFLAG.bit.RXC)
   {
      if (head != sioFifo[1].tail)
         sioFifo[1].data[sioFifo[1].head++] = sioPort.sercom[1]->USART.DATA.reg;
      else
         head = sioPort.sercom[1]->USART.DATA.reg; // clear the input buffer
   }
   sioFifo[1].head &= sizeof(sioFifo[1].data) - 1;                              // keep the pointer within the buffer address range
   sioPort.sercom[1]->USART.INTFLAG.reg = sioPort.sercom[1]->USART.INTFLAG.reg; // Clear interrupt flags
}

void SERCOM5_2_Handler(void) // Receive Complete Interrupt
{
   uint8_t head = (sioFifo[2].head + 1) & (sizeof(sioFifo[2].data) - 1);
   if (sioPort.sercom[2]->USART.INTFLAG.bit.RXC)
   {
      if (head != sioFifo[2].tail)
         sioFifo[2].data[sioFifo[2].head++] = sioPort.sercom[2]->USART.DATA.reg;
      else
         head = sioPort.sercom[2]->USART.DATA.reg; // clear the input buffer
   }
   sioFifo[2].head &= sizeof(sioFifo[2].data) - 1;                              // keep the pointer within the buffer address range
   sioPort.sercom[2]->USART.INTFLAG.reg = sioPort.sercom[2]->USART.INTFLAG.reg; // Clear interrupt flags
}