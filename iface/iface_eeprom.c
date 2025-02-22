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
 * @file cmd_llfs.c
 * @author Sergey Sanders
 * @date 21 Jun 2016
 * @brief Linked List File System (LLFS) file system commands for the parser
 * module FreeRTOS optimized
 *
 */
#include "FreeRTOS.h"
#include "llfs.h"
#include "llfs_vol_eeprom.h"
#include "rshell.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

cmd_err_t cmd_llfs_format(_cl_param_t *sParam);
cmd_err_t cmd_llfs_free(_cl_param_t *sParam);
cmd_err_t cmd_llfs_rm(_cl_param_t *sParam);
cmd_err_t cmd_llfs_ls(_cl_param_t *sParam);
cmd_err_t cmd_llfs_touch(_cl_param_t *sParam);
static bool iface_eeprom_init(bool verbose);

const _iface_t ifaceEEPROM = {
    .name = "eep",
    .prompt = NULL,
    .desc = "EEPROM llfs control",
    .init = iface_eeprom_init,
    .cmdList = {
        {.name = "format", .desc = "Format EEPROM", .func = cmd_llfs_format},
        {.name = "df", .desc = "Get free space", .func = cmd_llfs_free},
        {.name = "rm", .desc = "Remove file", .func = cmd_llfs_rm},
        {.name = "ls", .desc = "List EEPROM files", .func = cmd_llfs_ls},
        {.name = "touch", .desc = "Create a file", .func = cmd_llfs_touch},
        {.name = NULL, .func = NULL},
    }};

static bool iface_eeprom_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;
   initialized = true;
   if (verbose)
      tprintf("Interface \"EEPROM\" initialized.\n");
   return true;
}

cmd_err_t cmd_llfs_format(_cl_param_t *sParam)
{
   uint32_t size;
   tprintf("Formatting EEPROM.\n");
   lf_format(LLFS_EEPROM_SIZE, LLFS_DEVID_EEPROM, "EEPROM");
   if (lf_error == LF_ERR_NONE)
   {
      size = lf_get_free(0);
      tprintf("Done. %d bytes available\n", size);
   }
   else
   {
      tprintf("Format is not completed. Errno: %d\n", lf_error);
   }
   return CMD_NO_ERR;
}

cmd_err_t cmd_llfs_free(_cl_param_t *sParam)
{
   uint32_t size = lf_get_free(0);
   if (!lf_error)
   {
      tprintf(" %d bytes available\n", size);
   }
   else
   {
      tprintf("Unknown size. Errno: %d\n", lf_error);
   }
   return CMD_NO_ERR;
}

cmd_err_t cmd_llfs_rm(_cl_param_t *sParam)
{
   if (sParam->argc)
   {
      lf_delete(sParam->argv[0]);
      if (lf_error)
      {
         tprintf("Unable to delete %s\n", sParam->argv[0]);
      }
   }
   else
   {
      tprintf("No filename!\n");
   }
   return CMD_NO_ERR;
}

cmd_err_t cmd_llfs_ls(_cl_param_t *sParam)
{
   uint8_t next = 0;
   uint32_t fsize;
   char fName[13];
   lf_record_t record;
   while (
       (lf_find_record(sParam->argc ? sParam->argv[0] : "*", &record, next)))
   {
      next = 1;
      fsize = lf_get_fsize("", record.fptr);
      tprintf("%d %s\n", fsize, lf_rname_tostr(fName, record.name));
   }
   if (lf_error && (lf_error != LF_ERR_NOTFOUND))
      tprintf("ls error %d\n", lf_error);
   return CMD_NO_ERR;
}

cmd_err_t cmd_llfs_touch(_cl_param_t *sParam)
{
   lfile_t *file;
   if (!(file =
             lf_open(sParam->argv[0], MODE_WRITE | MODE_CREATE | MODE_APPEND)))
      tprintf("Can't touch %s. errno=%d\n", sParam->argc ? sParam->argv[0] : "empty file", lf_error);
   else
      tprintf("pos: %4x\n", file->index);
   lf_close(file);
   return CMD_NO_ERR;
}
