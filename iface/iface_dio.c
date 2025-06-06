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
 * @date September 2022
 * @brief * rshell interface "Digital IO"
 *   system control interface
 *
 */
#include "FreeRTOS.h"
#include "bsp.h"
#include "keyboard.h"
#include "rshell.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "enums.h"
#include "iface_dio.h"
#include "llfs.h"

bool io_conf_load(void);
bool io_conf_save(void);
static bool iface_dio_init(bool verbose);
static cmd_err_t cmd_dio_channel(_cl_param_t *sParam);
static cmd_err_t cmd_dio_dir(_cl_param_t *sParam);
static cmd_err_t cmd_dio_pin(_cl_param_t *sParam);
static cmd_err_t cmd_dio_pull(_cl_param_t *sParam);
cmd_err_t cmd_io_power(_cl_param_t *sParam);
cmd_err_t cmd_ioconf_save(_cl_param_t *sParam);

const uint8_t DioPinLut[2][6] = {{16, 17, 18, 19, 20, 21}, {12, 13, 14, 15, 0, 1}};
static char dioPromptBuff[8];
_ioconf_t ioConf;

const _iface_t ifaceDIO =
    {
        .name = "dio",
        .prompt = dioPromptBuff,
        .desc = "Digital IO ports control",
        .init = iface_dio_init,
        .cmdList =
            {
                {.name = "chan", .desc = "set active channel 0/1", .func = cmd_dio_channel},
                {.name = "pwr", .desc = "pwr : \'0\'=OFF, \'1\'=ON", .func = cmd_io_power},
                {.name = "dir", .desc = "Set dir [pin] [val]", .func = cmd_dio_dir},
                {.name = "pin", .desc = "Set pin [pin] [val]", .func = cmd_dio_pin},
                {.name = "pull", .desc = "Set pull (0-down,1-up) [pin] [val]", .func = cmd_dio_pull},
                {.name = "save", .desc = "Save config", .func = cmd_ioconf_save},
                {.name = NULL, .func = NULL},
            }};

static bool iface_dio_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;
   for (uint8_t pin = 0; pin < 6; pin++)
   {
      DIO0_PORT.PINCFG[DioPinLut[0][pin]].reg |= PORT_PINCFG_INEN; // | PORT_PINCFG_DRVSTR;
      DIO1_PORT.PINCFG[DioPinLut[1][pin]].reg |= PORT_PINCFG_INEN; // | PORT_PINCFG_DRVSTR;
   }
   PWR_PORT.OUTSET.reg = PWR_DIO1_EN | PWR_DIO0_EN;
   PWR_PORT.DIRSET.reg = PWR_DIO1_EN | PWR_DIO0_EN;
   PWR_UART_PORT.OUTSET.reg = PWR_UART_EN;
   PWR_UART_PORT.DIRSET.reg = PWR_UART_EN;
   if (io_conf_load())
   {
    initialized = true;
   }
   if (verbose)
      tprintf("Interface \"io\" initialized.\n");
   tsprintf((char *)ifaceDIO.prompt, "dio:%d", ioConf.dioChan);
   return initialized;
}

bool io_conf_load(void)
{
    _ioconf_t tmpConf;
    lfile_t *confFile;
    if ((confFile=lf_open(IO_CONF_FILE_NAME,MODE_READ)) != NULL)
    {
        lf_read(confFile,&tmpConf,sizeof(tmpConf));
        if (crc8((uint8_t *)&tmpConf,sizeof(tmpConf)-1) != tmpConf.checkSum)
            {
                memset(&tmpConf,0x00,sizeof(tmpConf));
                tmpConf.checkSum = crc8((uint8_t *)&tmpConf,sizeof(tmpConf)-1);
                lf_write(confFile,&ioConf,sizeof(tmpConf));
            }
        else
            memcpy((uint8_t *)&ioConf,(uint8_t *)&tmpConf,sizeof(_ioconf_t));
        lf_close(confFile);
    }
    else
    {
        if ((confFile = lf_open(IO_CONF_FILE_NAME,MODE_CREATE | MODE_WRITE)) == NULL) return false;
        lf_write(confFile,&ioConf,sizeof(ioConf));
        lf_close(confFile);
    }
    return true;
}

bool io_conf_save(void)
{
    lfile_t *confFile;
    ioConf.checkSum = crc8((uint8_t *)&ioConf,sizeof(ioConf)-1);
    if ((confFile = lf_open(IO_CONF_FILE_NAME,MODE_CREATE | MODE_WRITE)))
    {
        if (lf_write(confFile,(uint8_t *)&ioConf,sizeof(ioConf)))
            {
                lf_close(confFile);
                return true;
            }
          lf_close(confFile);  
    }
    return false;
}

static cmd_err_t cmd_dio_channel(_cl_param_t *sParam)
{
   if (sParam->argc)
   {
      ioConf.dioChan = ((uint8_t)strtol(sParam->argv[0], NULL, 0));
      if (ioConf.dioChan > 2)
         ioConf.dioChan = 2;
      tsprintf((char *)ifaceDIO.prompt, "dio:%d", ioConf.dioChan & 0x03);
   }
   else
      tprintf("%d\n", ioConf.dioChan);
   return CMD_NO_ERR;
}

bool set_io_power(uint8_t chan, bool stat)
{
   if (chan > 2) return false;
   if (chan == 2)
   {
      if (stat) PWR_UART_PORT.OUTCLR.reg = PWR_UART_EN;
      else PWR_UART_PORT.OUTSET.reg = PWR_UART_EN;
   }
   else
   {
      if (stat) PWR_PORT.OUTCLR.reg = chan ? PWR_DIO1_EN : PWR_DIO0_EN;
      else PWR_PORT.OUTSET.reg = chan ? PWR_DIO1_EN : PWR_DIO0_EN;
   }
   ioConf.power = stat ? ioConf.power | 0x01 << chan : ioConf.power & ~(0x01 << chan);
   return true;
}

cmd_err_t cmd_io_power(_cl_param_t *sParam)
{
   uint8_t chan = ioConf.dioChan;
   switch (sParam->argc)
   {
   case 0:
      tprintf("%s\n", (ioConf.dioChan == 2 ? PWR_UART_PORT.OUT.reg & PWR_UART_EN : PWR_PORT.OUT.reg & (ioConf.dioChan ? PWR_DIO1_EN : PWR_DIO0_EN)) ? "OFF" : "ON");
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

static cmd_err_t cmd_dio_dir(_cl_param_t *sParam)
{
   uint8_t pinNum = 0;
   uint8_t pinDir = 0;
   uint8_t chan = ioConf.dioChan;
   char *b;
   uint32_t shedReg;
   switch (sParam->argc)
   {
   case 3: // get channel
      chan = (uint8_t)strtol(sParam->argv[2], NULL, 0) ? 1 : 0;
   case 2:                              // get direction
      b = strchr(sParam->argv[1], 'b'); // binary
      if (b)
         pinDir = (uint8_t)strtol((char *)(b + 1), NULL, 2);
      else
         pinDir = (uint8_t)strtol(sParam->argv[1], NULL, 0);
   case 1: // get pin number
      pinNum = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      // set direction
      ioConf.dir[chan] = (pinNum < 6) ? (pinDir ? ioConf.dir[1] | (0x1 << pinNum) : ioConf.dir[1] & ~(0x1 << pinNum)) : (pinDir & 0x3f);
      if (chan)
      {
         shedReg = DIO1_PORT.DIR.reg & ~DIO1_PIN_MASK;
         shedReg |= ((ioConf.dir[1] & 0x0f) << 12) | ((ioConf.dir[1] >> 4) & 0x03);
         DIO1_PORT.DIR.reg = shedReg;
      }
      else
      {
         shedReg = DIO0_PORT.DIR.reg & ~DIO0_PIN_MASK;
         shedReg |= ((ioConf.dir[0] & 0x3f) << 16);
         DIO0_PORT.DIR.reg = shedReg;
      }
   case 0:
      shedReg = chan ? DIO1_PORT.DIR.reg : DIO0_PORT.DIR.reg;
      pinDir = chan ? ((shedReg >> 12) & 0x0f) | ((shedReg & 0x03) << 4) : ((shedReg >> 16) & 0x3f);
      break;
   default:
      return CMD_MISSING_PARAM;
   }
   tprintf("0x%2x\n", pinDir);
   return CMD_NO_ERR;
}

static cmd_err_t cmd_dio_pin(_cl_param_t *sParam)
{
   uint8_t pinNum = 0;
   uint8_t pin = 0;
   uint8_t chan = ioConf.dioChan;
   char *b;
   uint32_t shedReg;
   switch (sParam->argc)
   {
   case 3: // get channel
      chan = (uint8_t)strtol(sParam->argv[2], NULL, 0) ? 1 : 0;
   case 2:                              // get pin value
      b = strchr(sParam->argv[1], 'b'); // binary
      if (b)
         pin = (uint8_t)strtol((char *)(b + 1), NULL, 2);
      else
         pin = (uint8_t)strtol(sParam->argv[1], NULL, 0);
   case 1: // get pin number
      pinNum = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      // set pin
      ioConf.pin[chan] = (pinNum < 6) ? (pin ? ioConf.pin[1] | (0x1 << pinNum) : ioConf.pin[1] & ~(0x1 << pinNum)) : (pin & 0x3f);
      if (chan)
      {
         shedReg = DIO1_PORT.OUT.reg & ~DIO1_PIN_MASK;
         shedReg |= ((ioConf.pin[1] & 0x0f) << 12) | ((ioConf.pin[1] >> 4) & 0x03);
         DIO1_PORT.OUT.reg = shedReg;
      }
      else
      {
         shedReg = DIO0_PORT.OUT.reg & ~DIO0_PIN_MASK;
         shedReg |= ((ioConf.pin[0] & 0x3f) << 16);
         DIO0_PORT.OUT.reg = shedReg;
      }
   case 0:
      shedReg = chan ? DIO1_PORT.IN.reg : DIO0_PORT.IN.reg;
      pin = chan ? ((shedReg >> 12) & 0x0f) | ((shedReg & 0x03) << 4) : ((shedReg >> 16) & 0x3f);
      break;
   default:
      return CMD_MISSING_PARAM;
   }
   tprintf("0x%2x\n", pin);
   return CMD_NO_ERR;
}

static cmd_err_t cmd_dio_pull(_cl_param_t *sParam)
{
   uint8_t pinNum = 0;
   uint8_t pull = 0;
   uint8_t chan = ioConf.dioChan;
   char *b;
   uint32_t shedReg;
   switch (sParam->argc)
   {
   case 3: // get channel
      chan = (uint8_t)strtol(sParam->argv[2], NULL, 0) ? 1 : 0;
   case 2:                              // get pull value
      b = strchr(sParam->argv[1], 'b'); // binary
      if (b)
         pull = (uint8_t)strtol((char *)(b + 1), NULL, 2);
      else
         pull = (uint8_t)strtol(sParam->argv[1], NULL, 0);
   case 1: // get pin number
      pinNum = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      // set pin
      ioConf.pull[chan] = (pinNum < 6) ? (pull ? ioConf.pull[chan] | (0x1 << pinNum) : ioConf.pull[chan] & ~(0x1 << pinNum)) : (pull & 0x3f);
      if (pinNum > 5)
         for (uint8_t pin = 0; pin < 6; pin++)
            DIO_PORT.PINCFG[DioPinLut[chan][pinNum]].bit.PMUXEN = ioConf.pull[chan] & (0x01 << pin);
      else
         DIO_PORT.PINCFG[DioPinLut[chan][pinNum]].bit.PMUXEN = ioConf.pull[chan] & (0x01 << pull);
   case 0:
      shedReg = DIO_PORT.IN.reg;
      pull = chan ? ((shedReg >> 12) & 0x0f) | ((shedReg & 0x03) << 4) : ((shedReg >> 16) & 0x3f);
      break;
   default:
      return CMD_MISSING_PARAM;
   }
   tprintf("0x%2x\n", pull);
   return CMD_NO_ERR;
}

cmd_err_t cmd_ioconf_save(_cl_param_t *sParam)
{
    if (!io_conf_save()) return "Conf save failed!"; 
        tprintf("Config saved.\n");
   return CMD_NO_ERR;
}