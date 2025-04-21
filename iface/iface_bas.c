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
 * @file iface_mem.c
 * @author Sergey Sanders
 * @date September 2022
 * @brief * rshell interface "Memory"
 *   memory control interface
 *
 */
#include "FreeRTOS.h"
#include "bcore.h"
#include "bedit.h"
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

static cmd_err_t bas_load(_cl_param_t *sParam);
static cmd_err_t bas_run(_cl_param_t *sParam);
static cmd_err_t bas_new(_cl_param_t *sParam);
static cmd_err_t cmd_bas_list(_cl_param_t *sParam);
static cmd_err_t bas_vars(_cl_param_t *sParam);
static bool iface_bas_init(bool verbose);

const _iface_t ifaceBasic =
    {
        .name = "bas",
        .prompt = NULL,
        .desc = "BasicD interpreter",
        .init = iface_bas_init,
        .cmdList =
            {
                {.name = "load", .desc = "Lload", .func = bas_load},
                {.name = "run", .desc = "Run", .func = bas_run},
                {.name = "new", .desc = "New", .func = bas_new},
                {.name = "list", .desc = "List", .func = cmd_bas_list},
                {.name = "var", .desc = "List variables", .func = bas_vars},
                {.name = "bas", .desc = "Run the interpreter", .func = basic_exe},
                {.name = NULL, .func = NULL},
            }};

static bool iface_bas_init(bool verbose)
{
   if (verbose)
   {
      tprintf(ANSI_CLS);
      tprintf("Basic D\n Version 0.2b\n");
      tprintf("Use \"bas\" for the interpreter\n");
   }
   return true;
}

static cmd_err_t bas_load(_cl_param_t *sParam)
{
   prog_load(sParam->argc ? sParam->argv[0] : "");
   if (sParam->argc > 1)
      prog_run((uint16_t)strtol(sParam->argv[1], NULL, 10));
   return CMD_NO_ERR;
}

static cmd_err_t bas_run(_cl_param_t *sParam)
{
   uint16_t lineNum = 0;
   if (sParam->argc)
      lineNum = (uint16_t)strtol(sParam->argv[0], NULL, 10);
   prog_run(lineNum);
   return CMD_NO_ERR;
}

static cmd_err_t bas_new(_cl_param_t *sParam)
{
   __new(NULL);
   return CMD_NO_ERR;
}

static cmd_err_t cmd_bas_list(_cl_param_t *sParam)
{
   void bas_list(uint16_t, uint16_t);
   _stream_io_t *lastStream = stdio;
   vTaskSuspend(xuTermTask);
   vTaskDelay(50);
   stdio = &basicStream;
   bas_list(0, 1000);
   stdio = lastStream;
   vTaskResume(xuTermTask);
   return CMD_NO_ERR;
}

static cmd_err_t bas_vars(_cl_param_t *sParam)
{
   extern _bas_var_t *BasicVars;
   _bas_var_t *var = BasicVars;
   while (var)
   {
      tprintf("name: %s; type 0x%2x\n", var->name, var->value.type);
      var = var->next;
   }
   return CMD_NO_ERR;
}