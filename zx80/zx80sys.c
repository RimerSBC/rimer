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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "string.h"
#include "bsp.h"
#include "zxscreen.h"
#include "z80cpu.h"
#include "zx80sys.h"
#include "z80macros.h"
#include "z80user.h"

uint8_t *z80mem = NULL;

uint8_t borderRGB = 0;

volatile bool zx50HzSignal = true;
volatile uint16_t addrMatch = 0xFFFF;

TcCount16 *tmrZX50Hz = (TcCount16 *)TC1;

void int50Hz_init(void)
{
   REG_MCLK_APBAMASK |= MCLK_APBAMASK_TC1;            // enable TC1 clock
   REG_GCLK_PCHCTRL9 = CLK_60MHZ | GCLK_PCHCTRL_CHEN; // GCLK peripheral TC0 and TC1 clock @ 12MHz
   tmrZX50Hz->CTRLA.bit.PRESCALER = 0x05;             // 60MHz / 64 = 937.5KHz
   tmrZX50Hz->CC[0].reg = 18750 + 300;                // 937.5KHz / 18750 = 50Hz
   tmrZX50Hz->INTENSET.bit.OVF = 1;                   // enable interrupt
   tmrZX50Hz->INTENSET.bit.MC0 = 1;                   // enable interrupt
   tmrZX50Hz->WAVE.reg = 0x01;                        // Match compare
   tmrZX50Hz->CTRLA.bit.ENABLE = 1;
   vTaskDelay(1);
   tmrZX50Hz->CTRLBSET.bit.CMD = 0x02; // stop the timer
   NVIC_EnableIRQ(TC1_IRQn);
}
void int50Hz_start(void)
{
   tmrZX50Hz->CTRLBSET.bit.CMD = 0x01; // start the timer
}
void int50Hz_stop(void)
{
   tmrZX50Hz->CTRLBSET.bit.CMD = 0x02; // stop the timer
}

void __attribute__((long_call, section(".ramfunc"), optimize("3"))) Z80Interrupt(void)
{
   z80state.status = 0;
   zx50HzSignal = true;
   if (z80state.iff1)
   {
      z80state.iff1 = z80state.iff2 = 0;
      z80state.r = (z80state.r & 0x80) | ((z80state.r + 1) & 0x7f);
      switch (z80state.im)
      {
      case Z80_INTERRUPT_MODE_0:
      {
         /* Assuming the opcode in data_on_bus is an
          * RST instruction, accepting the interrupt
          * should take 2 + 11 = 13 cycles.
          */
         // tmrZ80Cpu->CC[0].reg = clkZ80div*13;
         asm("nop");
         break; // Not used in ZX Spectrum
      }
      case Z80_INTERRUPT_MODE_1:
      {
         SP -= 2;
         Z80_WRITE_WORD_INTERRUPT(SP, z80state.pc);
         z80state.pc = 0x0038;
         // tmrZ80Cpu->CC[0].reg = clkZ80div*13;
         break;
      }
      case Z80_INTERRUPT_MODE_2:
      default:
      {
         uint16_t vector;
         SP -= 2;
         Z80_WRITE_WORD_INTERRUPT(SP, z80state.pc);
         vector = ((uint16_t)z80state.i) << 8 | dataOnBus;
#ifdef Z80_MASK_IM2_VECTOR_ADDRESS
         vector &= 0xfffe;
#endif
         z80state.pc = Z80_READ_WORD_INTERRUPT(vector);
      }
      }
   }
   CLEAR_Z80_INT_FLAGS();
}

uint8_t __attribute__((long_call, section(".ramfunc"), optimize("3"))) z80sys_input(uint16_t port)
// uint8_t __attribute__((long_call, section(".ramfunc"), optimize("0"))) z80sys_input(uint16_t port)
{
   uint8_t micBit;
   uint8_t hPort;
   switch ((uint8_t)port)
   {
   case 0x3b:   // USART
      return 0; //(uint8_t)uartPort->USART.DATA.reg;
      break;
      /**
          IN:    Reads keys (bit 0 to bit 4 inclusive)

            0xfefe  SHIFT, Z, X, C, V            0xeffe  0, 9, 8, 7, 6
            0xfdfe  A, S, D, F, G                0xdffe  P, O, I, U, Y
            0xfbfe  Q, W, E, R, T                0xbffe  ENTER, L, K, J, H
            0xf7fe  1, 2, 3, 4, 5                0x7ffe  SPACE, SYM SHFT, M, N, B

      */
   case 0xfe:        // KEYBOARD and EAR input port
      micBit = 0xff; // 0xBF; // TODO: need to connect to real port
      hPort = port >> 8;
      addWaitStates = 4;
      if (hPort >= 0x40 && hPort <= 0x7f && !(port & 0x01))
         addWaitStates = 3;
      for (uint8_t i = 0; i < 8; i++, hPort >>= 1)
         if (!(hPort & 0x01))
            micBit &= keyRows[i];
      if (!AC->STATUSA.bit.STATE0) // read Aanalog Comparator 0 output
      {
         micBit &= ~(0x1 << 6);
         DAC->DATA[1].reg = 0;
      }
      else
         DAC->DATA[1].reg = 500;
      return micBit;
      break;
   }
   return 0xff;
}

void __attribute__((long_call, section(".ramfunc"), optimize("3"))) z80sys_output(uint16_t port, uint8_t data)
{
   switch ((uint8_t)port)
   {
   case 0x3b: // UART
      break;
   case 0xfe: // ear, mic and border
      uint8_t hPort = (port >> 8);
      addWaitStates = 4;
      if (hPort >= 0x40 && hPort <= 0x7f && !(port & 0x01))
         addWaitStates = 3;
      if (data & 0x10)
         DAC->DATA[1].reg = sysConf.volume;
      else
         DAC->DATA[1].reg = 0;
      if (data & 0x08)
         DAC->DATA[0].reg = 2000; // AIO_PIN_DACOUT 0-1.5V
      else
         DAC->DATA[0].reg = 0;
      borderRGB = ZxColour[0][data & 0x07]; // set border colour
      break;
   }
}

/*
void joystick_init(void)
{
    uint8_t initData;
    SERCOM_JS_CONFIG();
    i2c_port_init(JS_SERCOM,I2C_SPEED_400KHZ);
    initData = 0x55;
    if(i2c_write(JS_SERCOM,WII_ADDRESS,0x00f0,&initData,1) != I2C_ERR_NONE)
    {
        tprintf("\nJoystick not found.\n");
        return;
    }
    vTaskDelay(10);
    initData = 0x00;
    i2c_write(JS_SERCOM,WII_ADDRESS,0x00fb,&initData,1);
    vTaskDelay(20);
    initData = 0x01;
    i2c_write(JS_SERCOM,WII_ADDRESS,0x00fe,&initData,1); // request datatype 3
}

uint8_t joystick_get(void)
{

}*/
