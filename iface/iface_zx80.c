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
 * @file iface_zx80.c
 * @author Sergey Sanders
 * @date September 2022
 * @brief * rshell interface "zx80"
 *   ZX Spectrum emulator control
 *
 */
#include "FreeRTOS.h"
#include <stdlib.h>
#include "bsp.h"
#include "ff.h"
#include "keyboard.h"
#include "rshell.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include "zx80sys.h"
#include "z80mnx.h"
#include "zxscreen.h"
#include "snapshot.h"
#include "z80dbg.h"

#define ZX_ROM_DIR "/zx80"               // all emulator's files will be located here
#define ZX_ROM_FILE "rom48.bin"          // ZX Spectrum 48k ROM file
#define ZX_TEST_ROM_FILE "zxTestRom.rom" // test ROM file
char path[256];
static bool iface_zx80_init(bool verbose);
static cmd_err_t zx_zx(_cl_param_t *sParam);
static cmd_err_t zx_load(_cl_param_t *sParam);
static cmd_err_t zx_dbg(_cl_param_t *sParam);

const _iface_t ifaceZX80 =
    {
        .name = "zx",
        .prompt = NULL,
        .desc = "ZX Spectrum Emulator",
        .init = iface_zx80_init,
        .cmdList =
            {
                {.name = "zx", .desc = "Start emulator", .func = zx_zx},
                {.name = "load", .desc = "Load program", .func = zx_load},
                {.name = "dbg", .desc = "Start z80 debugger", .func = zx_dbg},
                {.name = NULL, .func = NULL},
            }};

static bool iface_zx80_init(bool verbose)
{
   if (verbose)
   {
      tprintf(ANSI_CLS);
      tprintf("ZX Spectrum emulator\n");
      tprintf("Use \"zx\" to run\n");
   }
   return true;
}

static bool zxInitialized = false;

bool zx_init(void)
{
   FIL RomFile;
   FRESULT fr;
   unsigned int bytesRead;
   if (!z80mem)
      z80mem = pvPortMalloc(65536);
   if (!z80mem)
   {
      tprintf("Can't allocate memory!\n");
      return false;
   }
   if ((fr = f_chdir(ZX_ROM_DIR)) != FR_OK) /* Change directory */
   {
      tprintf("can't cd into %s [%d]\n", ZX_ROM_DIR, fr);
      return CMD_NO_ERR;
   }
   if ((f_open(&RomFile, ZX_ROM_FILE, FA_READ) != FR_OK))
   {
      tprintf("File sd:%s not found!\n", ZX_ROM_FILE);
      return false;
   }
   if (f_read(&RomFile, z80mem, 16384, &bytesRead) != FR_OK)
   {
      tprintf("Error reading ROM file\n");
      f_close(&RomFile);
      return false;
   }
   f_close(&RomFile);
   zxInitialized = true;
   vTaskDelay(100);
   int50Hz_init();
   xTaskCreate(lcd_zx_task, "lcdZx", configMINIMAL_STACK_SIZE, NULL, 2, &xLcdZxTask);
   z80_init();
   z80state.pc = 0x0000;
   /// Physical ports init
   /// Enable speaker (keep analog out)
   /// Enable MIC out (keep analog out)
   /// Enable EAR in (set Analog Comparator in)
   REG_GCLK_PCHCTRL32 = CLK_12MHZ | GCLK_PCHCTRL_CHEN; // AC clock @ 12MHz
   REG_MCLK_APBCMASK |= MCLK_APBCMASK_AC;
   AIO_PORT0.WRCONFIG.reg = CONF_PIN_VAL(AIO_PIN_ADCIN) | PORT_WRCONFIG_PMUX(0x01) | PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_WRPINCFG;
   AC->SCALER[0].reg = 11; // 0.52V @ 3V Vdd
   AC->COMPCTRL[0].reg = AC_COMPCTRL_ENABLE | AC_COMPCTRL_MUXNEG_VSCALE;
   AC->CTRLA.bit.ENABLE = 1;
   vTaskDelay(100);
   return true;
}

static cmd_err_t zx_zx(_cl_param_t *sParam)
{
   if (!zxInitialized)
   {
      if (!zx_init())
         return "Failed to init ZX system!";
   }
   if (sParam->argc)
      z80state.pc = (uint16_t)strtol(sParam->argv[0], NULL, 0);
   vTaskSuspend(xuTermTask);
   zxKeyboard = true;
   z80cpu_run();
   vTaskDelay(10);
   keyboard_break(); // clear kbd break flag
   while (!keyboard_break())
      taskYIELD();
   z80cpu_stop();
   zxKeyboard = false;
   vTaskDelay(60);
   keyboard_flush();
   vTaskResume(xuTermTask);
   text_cls();
   return CMD_NO_ERR;
}

static cmd_err_t zx_load(_cl_param_t *sParam)
{
   enum
   {
      SNAP_TYPE_NONE,
      SNAP_TYPE_Z80,
      SNAP_TYPE_SNA,
   } fType = SNAP_TYPE_NONE;
   FIL progFile;
   unsigned int bytesRead;
   char *ext = sParam->argv[0];
   if (!sParam->argc)
   {
      return CMD_MISSING_PARAM;
   }

   while (*ext && (*ext != '.'))
      ext++;
   if (*ext++)
   {
      if (!strcmp(ext, "z80"))
         fType = SNAP_TYPE_Z80;
      else if (!strcmp(ext, "Z80"))
         fType = SNAP_TYPE_Z80;
      else if (!strcmp(ext, "sna"))
         fType = SNAP_TYPE_SNA;
      else if (!strcmp(ext, "SNA"))
         fType = SNAP_TYPE_SNA;
   }
   if (fType == SNAP_TYPE_NONE)
      return "Unsupported file type!";

   if (!zxInitialized)
   {
      if (!zx_init())
         return CMD_NO_ERR;
   }

   if ((f_open(&progFile, sParam->argv[0], FA_READ) != FR_OK))
   {
      tprintf("File sd:%s not found!\n", sParam->argv[0]);
      return CMD_NO_ERR;
   }

   zxKeyboard = true;
   if (f_read(&progFile, frameBuffer, FB_SIZE, &bytesRead) != FR_OK) // using frame buffer for temporary storage
   {
      // tprintf("Error reading prog file\n");
      f_close(&progFile);
      return CMD_NO_ERR;
   }
   f_close(&progFile);
   for (uint8_t i = 0; i < 8; i++)
      keyRows[i] = 0xff;
   if (fType == SNAP_TYPE_Z80)
      load_snapshot_z80(frameBuffer);
   else
      load_snapshot_sna(frameBuffer);
   lcd_cls(0);
   vTaskSuspend(xuTermTask);
   z80cpu_run();
   int50Hz_start();
   vTaskDelay(10);
   keyboard_break(); // clear kbd break flag
   while (!keyboard_break())
      taskYIELD();
   z80cpu_stop();
   zxKeyboard = false;
   vTaskDelay(60);
   keyboard_flush();
   vTaskResume(xuTermTask);
   text_cls();
   return CMD_NO_ERR;
}

static cmd_err_t zx_dbg(_cl_param_t *sParam)
{
   if (!zxInitialized)
   {
      if (!zx_init())
         return CMD_NO_ERR;
   }
   uint16_t addr = z80state.pc;
   if (sParam->argc)
      addr = (uint16_t)strtol(sParam->argv[0], NULL, 0);
   z80dbg(addr);
   vTaskDelay(60);
   keyboard_flush();
   return CMD_NO_ERR;
}