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
 * @brief * rshell interface "Setup"
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

#include "fat.h"
#include "ff.h"
#include "sd_mmc.h"

static bool iface_sd_init(bool verbose);
static cmd_err_t cmd_fat_sdi(_cl_param_t *sParam);
static cmd_err_t cmd_fat_block_read(_cl_param_t *sParam);
static cmd_err_t cmd_fat_ls(_cl_param_t *sParam);
static cmd_err_t cmd_fat_cd(_cl_param_t *sParam);
static cmd_err_t cmd_fat_pwd(_cl_param_t *sParam);
static cmd_err_t cmd_fat_rm(_cl_param_t *sParam);

const _iface_t ifaceSD =
    {
        .name = "sd",
        .prompt = NULL,
        .desc = "sd card FAT control",
        .init = iface_sd_init,
        .cmdList =
            {
                {.name = "sdi", .desc = "sd card info", .func = cmd_fat_sdi},
                {.name = "sdrd", .desc = "sd card read : block count", .func = cmd_fat_block_read},
                {.name = "ls", .desc = "directory list", .func = cmd_fat_ls},
                {.name = "cd", .desc = "change directory", .func = cmd_fat_cd},
                {.name = "pwd", .desc = "get working directory", .func = cmd_fat_pwd},
                {.name = "rm", .desc = "remove file(s)", .func = cmd_fat_rm},
                {.name = NULL, .func = NULL},
            }};

static bool iface_sd_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;
   initialized = true;
   if (verbose)
      tprintf("Interface \"sd\" initialized.\n");
   return true;
}

cmd_err_t cmd_fat_sdi(_cl_param_t *sParam)
{
   uint8_t slot = 0;
   if (sParam->argc)
   {
      slot = (uint8_t)strtol(sParam->argv[0], NULL, 0);
      if (slot > 1)
         slot = 1;
   }
   sd_info_print(slot);
   return CMD_NO_ERR;
}

cmd_err_t cmd_fat_block_read(_cl_param_t *sParam)
{
   uint32_t block = 0;
   uint16_t count = 1;
   uint8_t line = 0;
   if (sParam->argc)
   {
      block = (uint32_t)strtol(sParam->argv[0], NULL, 0);
      if (sParam->argc > 1)
         count = (uint16_t)strtol(sParam->argv[1], NULL, 0);
   }
   while (SD_MMC_OK != sd_mmc_check(0))
      taskYIELD(); // Wait card ready.

   if (sd_mmc_get_type(0) & (CARD_TYPE_SD | CARD_TYPE_MMC))
      for (uint16_t i = block; i < block + count; i++)
      {
         sd_mmc_init_read_blocks(0, i, 1);
         sd_mmc_start_read_blocks(sd_mmc_block, 1);
         sd_mmc_wait_end_of_read_blocks(false);
         for (uint16_t j = 0; j < sizeof(sd_mmc_block); j += 8)
            dump8(j, &sd_mmc_block[j], 8, &line);
      }
   else
      tprintf("No SD card.\n");
   return CMD_NO_ERR;
}

cmd_err_t cmd_fat_ls(_cl_param_t *sParam)
{
   enum
   {
      ALL = 0x01,  // do not ignore entries starting with .
      DIRS = 0x02, // show directories only
      LIST = 0x04, // use a long listing format
   };
   FRESULT fr;  /* Return value */
   DIR dj;      /* Directory object */
   FILINFO fno; /* File information */
   uint8_t lsArgs = 0, argPtr = 0;
   uint8_t lineCnt = 0;
   if (sParam->argc && (*sParam->argv[0] == '-'))
   {
      char *argStr = (char *)sParam->argv[0];
      argPtr = 1;
      while (*++argStr)
         switch (*argStr)
         {
         case 'a':
            lsArgs |= ALL;
            break;
         case 'd':
            lsArgs |= DIRS;
            break;
         case 'l':
            lsArgs |= LIST;
            break;
         }
   }
   if ((fr = f_findfirst(&dj, &fno, "", (sParam->argc > argPtr) ? sParam->argv[argPtr] : "*")) != FR_OK) /* Start to search for files */
   {
      tprintf("ls error [%d]\n", fr);
      return CMD_ERR_EMPTY;
   }
   while (fr == FR_OK && fno.fname[0]) /* Repeat while an item is found */
   {
      if (lineCnt > uTerm.lines - 2)
      {
         tprintf("\e[a\e[33mscroll?\e[r");
         if (keyboard_wait("yY"))
            lineCnt = 0;
         tprintf("\e[1K\r");
         if (lineCnt)
            break;
      }
      if ((*fno.fname != '.') || (lsArgs & ALL))
      {
         if (fno.fattrib & AM_DIR)
         {
            tprintf("[ %s ]\n", fno.fname); /* Print directory name */
            lineCnt++;
         }
         else if (!(lsArgs & DIRS))
         {
            if (lsArgs & LIST)
            {
                tprintf("%2x %d ",fno.fattrib, fno.fsize);
            }
            tprintf("%s", fno.fname); /* Print the object name */
            tprintf("\n");
            lineCnt++;
         }
      }
      fr = f_findnext(&dj, &fno); /* Search for next item */
   }
   f_closedir(&dj);
   return CMD_NO_ERR;
}

cmd_err_t cmd_fat_cd(_cl_param_t *sParam)
{
   FRESULT fr; /* Return value */
   if (!sParam->argc)
      return CMD_MISSING_PARAM;
   if ((fr = f_chdir(sParam->argv[0])) != FR_OK) /* Change directory */
   {
      tprintf("can't cd into %s [%d]\n", sParam->argv[0], fr);
   }
   return CMD_NO_ERR; 
}

cmd_err_t cmd_fat_pwd(_cl_param_t *sParam)
{
   FRESULT fr; /* Return value */
   char str[64];
   if (sParam->argc)
      return CMD_MISSING_PARAM;
   if ((fr = f_getcwd(str, sizeof(str))) != FR_OK) /* Get current directory */
   {
      tprintf("can't read directory into [%d]\n", sParam->argv[0], fr);
      return CMD_ERR_EMPTY;
   }
   tprintf("%s\n", str);
   return CMD_NO_ERR;
}

cmd_err_t cmd_fat_rm(_cl_param_t *sParam)
{
   FRESULT fr;  // Return value
   DIR dj;      // Directory object
   FILINFO fno; // File information
   if (!sParam->argc)
   {
      return CMD_MISSING_PARAM;
   }
   if ((fr = f_findfirst(&dj, &fno, "", sParam->argv[0])) != FR_OK) // Start to search for files
   {
      tprintf("ls error [%d]\n", fr);
      return CMD_ERR_EMPTY;
   }
   if (strchr(sParam->argv[0], '*'))      // massive erase
      while (fr == FR_OK && fno.fname[0]) // Repeat while an item is found
      {
         f_unlink(fno.fname);        // delete file
         fr = f_findnext(&dj, &fno); // Search for next item
      }
   else if (!strcmp(fno.fname, sParam->argv[0]))
   {
      tprintf("\e[a\e[33mdelete %s(y/n)?\e[r", sParam->argv[0]);
      if (keyboard_wait("Yy"))
         f_unlink(sParam->argv[0]);
   }
   stdio->putch('\n');
   f_closedir(&dj);
   return CMD_NO_ERR;
}