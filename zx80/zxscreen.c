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
 * @file lcd.c
 * @author Sergey Sanders
 * @date July 2020
 * @brief * LCD 320x240 control
 *
 */
#include "zxscreen.h"
#include "FreeRTOS.h"
#include "bsp.h"
#include "lcd.h"
#include "task.h"
#include "timers.h"
#include "zx80sys.h"
#include "keyboard.h"

/// LCD ZX section
#define ZX_SCREEN_WIDTH 256
#define ZX_SCREEN_HEIGHT 192

TaskHandle_t xLcdZxTask;

#define BASE_TONE 192
#define BRIGHT_TONE 255

const uint8_t ZxColour[2][8] =
    {
        {
            _rgb(0, 0, 0),
            _rgb(0, 0, BASE_TONE),
            _rgb(BASE_TONE, 0, 0),
            _rgb(BASE_TONE, 0, BASE_TONE),
            _rgb(0, BASE_TONE, 0),
            _rgb(0, BASE_TONE, BASE_TONE),
            _rgb(BASE_TONE, BASE_TONE, 0),
            _rgb(BASE_TONE, BASE_TONE, BASE_TONE),
        },
        {
            _rgb(0, 0, 0),
            _rgb(0, 0, BRIGHT_TONE),
            _rgb(BRIGHT_TONE, 0, 0),
            _rgb(BRIGHT_TONE, 0, BRIGHT_TONE),
            _rgb(0, BRIGHT_TONE, 0),
            _rgb(0, BRIGHT_TONE, BRIGHT_TONE),
            _rgb(BRIGHT_TONE, BRIGHT_TONE, 0),
            _rgb(BRIGHT_TONE, BRIGHT_TONE, BRIGHT_TONE),
        }};

uint8_t zxBorderColour = _rgb(BASE_TONE, BASE_TONE, BASE_TONE); // should be updated for every frame

// uint8_t *screen = (uint8_t *)ZxColour;

// 256 x 192
// left/right 32 pixels
// up/down 24 pixels
//  display coord:   32,24    287,24
//                   32,215   287,215

static uint8_t *screenMem;
static uint8_t *attrMem;

uint16_t attrColorTable[256];
uint8_t flash;        // 3Hz flash flag
TimerHandle_t xFlash; // 3 Hz flash timer
void on_flash_timer(TimerHandle_t xTimer)
{
   flash = flash ? 0x00 : 0x80;
}
#if 1
void __attribute__((long_call, section(".ramfunc"), optimize("3"))) lcd_zx_task(void *vParam)
{
   uint16_t i, j;
   uint8_t *lcdData;
   uint8_t *attr;
   uint8_t *byte;
   if ((xFlash = xTimerCreate("flash", 330, pdTRUE, (void *)0, on_flash_timer)) != NULL)
      xTimerStart(xFlash, 0);
   screenMem = (uint8_t *)(z80mem + 0x4000);
   attrMem = (uint8_t *)(z80mem + 0x5800);
   for (i = 0; i < 256; i++)
      attrColorTable[i] = ZxColour[(i & 0x40) ? 1 : 0][(i & 0x80) ? (i & 0x07) : ((i >> 3) & 0x07)];
   DIO0_PORT.DIRSET.reg = DIO0_PIN_WO1;
   while (1)
   {
    lcdData = frameBuffer;
    
     while (!zx50HzSignal)
        taskYIELD();
     zx50HzSignal = false;
      DIO0_PORT.OUTSET.reg = DIO0_PIN_WO1;
      for (j = 0; j < 24; j++) // in order to update the border's colour faster, update 160 pixes of the border's line at once (instead of 320), then yield the tasks.
      {
         for (i = 0; i < 320; i++)
            *lcdData++ = borderRGB;
/*         lcdData -= 128;
         for (i = 0; i < 128; i++)
            *lcdData++ = borderRGB;*/
      }
      for (uint8_t block = 0; block < 3; block++)
      {
         for (i = 0; i < 64; i++)
         {
            for (j = 0; j < 32; j++)
               *lcdData++ = borderRGB;
            attr = &attrMem[((i + block * 64) / 8 * 32)];
            byte = &screenMem[(block * 2048) + (((i & 0x07) * 256) + ((i >> 3) * 32))];
            for (uint8_t by = 0; by < 32; by++, attr++, byte++) // display horisontal line
            {
               uint8_t bb = (flash & *attr) ? ~*byte : *byte;
               for (uint8_t p = 0; p < 8; p++, bb <<= 1)
                  *lcdData++ = attrColorTable[(bb & 0x80) ? (*attr | 0x80) : (*attr & 0x7f)];
            }
            for (j = 0; j < 32; j++)
               *lcdData++ = borderRGB;
            //taskYIELD();
         }
      }
      for (j = 0; j < 24; j++)
      {
         for (i = 0; i < 320; i++)
            *lcdData++ = borderRGB;
         //taskYIELD();
      }
      DIO0_PORT.OUTCLR.reg = DIO0_PIN_WO1;
      vSync = true; // start LCD flush
      // kbdScanRow = true; // scan next keyboard row
      // taskYIELD();
      //vTaskSuspend(NULL);
   }
}

#else
void __attribute__((long_call, section(".ramfunc"), optimize("3"))) flush_zx_screen(void)
{
   uint16_t i, j;
   uint8_t *lcdData;
   uint8_t *attr;
   uint8_t *byte;
   lcdData = frameBuffer;
   for (j = 0; j < 24; j++) // in order to update the border's colour faster, update 160 pixes of the border's line at once (instead of 320), then yield the tasks.
   {
      for (i = 0; i < 320; i++)
         *lcdData++ = borderRGB;
      lcdData -= 128;
      for (i = 0; i < 128; i++)
         *lcdData++ = borderRGB;
   }
   for (uint8_t block = 0; block < 3; block++)
   {
      for (i = 0; i < 64; i++)
      {
         for (j = 0; j < 32; j++)
            *lcdData++ = borderRGB;
         attr = &attrMem[((i + block * 64) / 8 * 32)];
         byte = &screenMem[(block * 2048) + (((i & 0x07) * 256) + ((i >> 3) * 32))];
         for (uint8_t by = 0; by < 32; by++, attr++, byte++) // display horisontal line
         {
            uint8_t bb = (flash & *attr) ? ~*byte : *byte;
            for (uint8_t p = 0; p < 8; p++, bb <<= 1)
               *lcdData++ = attrColorTable[(bb & 0x80) ? (*attr | 0x80) : (*attr & 0x7f)];
         }
         for (j = 0; j < 32; j++)
            *lcdData++ = borderRGB;
         taskYIELD();
      }
   }
   for (j = 0; j < 80; j++)
   {
      for (i = 0; i < 96; i++)
         *lcdData++ = borderRGB;
      taskYIELD();
   }
}

// void __attribute__((long_call, section(".ramfunc"), optimize("3"))) lcd_zx_task(void *vParam)
void lcd_zx_task(void *vParam)
{
   uint16_t i;
   if ((xFlash = xTimerCreate("flash", 330, pdTRUE, (void *)0, on_flash_timer)) != NULL)
      xTimerStart(xFlash, 0);

   screenMem = (uint8_t *)(z80mem + 0x4000);
   attrMem = (uint8_t *)(z80mem + 0x5800);
   for (i = 0; i < 256; i++)
      attrColorTable[i] = ZxColour[(i & 0x40) ? 1 : 0][(i & 0x80) ? (i & 0x07) : ((i >> 3) & 0x07)];
   // lcd_vsync_mode(true); // enable synchronised LCD flush
   DIO0_PORT.DIRSET.reg = DIO0_PIN_WO1;
   while (1)
   {
      while (!zx50HzSignal)
         taskYIELD();
      zx50HzSignal = false;
      DIO0_PORT.OUTSET.reg = DIO0_PIN_WO1;
      flush_zx_screen();
      DIO0_PORT.OUTCLR.reg = DIO0_PIN_WO1;
      vSync = true;      // start LCD flush
      kbdScanRow = true; // scan next keyboard row
      taskYIELD();
   }
}
#endif