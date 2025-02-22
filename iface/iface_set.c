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
 *   system settings interface
 *
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "uterm.h"
#include "rshell.h"
#include "enums.h"
#include "tstring.h"
#include "sys_config.h"


static cmd_err_t set_foreground(_cl_param_t *sParam);
static cmd_err_t set_background(_cl_param_t *sParam);
static cmd_err_t set_volume(_cl_param_t *sParam);
static cmd_err_t set_brightness(_cl_param_t *sParam);
static cmd_err_t set_font(_cl_param_t *sParam);
static cmd_err_t set_startup(_cl_param_t *sParam);
static bool iface_set_init(bool verbose);

const _iface_t ifaceSetup =
    {
        .name = "set",
        .prompt = NULL,
        .desc = "System settings",
        .init = iface_set_init,
        .cmdList =
            {
                {.name = "fg", .desc = "Set text foreground color 0:15", .func = set_foreground},
                {.name = "bg", .desc = "Set text background color 0:15", .func = set_background},
                {.name = "bright", .desc = "Set LCD brightness in %", .func = set_brightness},
                {.name = "font", .desc = "Set system font (index)", .func = set_font},
                {.name = "vol", .desc = "Set sound volume in %", .func = set_volume},
                {.name = "startup", .desc = "Set start up command(s)", .func = set_startup},
                {.name = NULL, .func = NULL},
            }};

static bool iface_set_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;
   initialized = true;
   if (verbose)
      tprintf("Interface \"set\" initialized.\n");
   return true;
}

static cmd_err_t set_foreground(_cl_param_t *sParam)
{
   uint8_t colour = 0;
   if (sParam->argc)
       colour = tget_enum(sParam->argv[0],EnumColours);
   if (colour > 15)
      colour = 15;
   if (colour == sysConf.textBG)
   {
      sysConf.textBG = colour < 8 ? colour + 8 : colour - 8;
      text_bg_colour(sysConf.textBG);
   }
   sysConf.textFG = colour;
   text_fg_colour(colour);
   conf_save();
   return CMD_NO_ERR;
}

static cmd_err_t set_background(_cl_param_t *sParam)
{
   uint8_t colour = 0;
   if (sParam->argc)
      colour = tget_enum(sParam->argv[0],EnumColours);
   if (colour > 15)
      colour = 15;
   if (colour == sysConf.textFG)
   {
      sysConf.textFG = colour < 8 ? colour + 8 : colour - 8;
      text_fg_colour(sysConf.textFG);
   }
   sysConf.textBG = colour;
   text_bg_colour(colour);
   conf_save();
   return CMD_NO_ERR;
}

static cmd_err_t set_volume(_cl_param_t *sParam)
{
   uint8_t volume = 0;
   if (sParam->argc)
      volume = (uint8_t)strtol(sParam->argv[0], NULL, 10);
   if (volume > 100)
      volume = 100;
   sysConf.volume = volume * (DEF_CONF_SPKR_MAX / 100);
   tprintf("Volume: %d%%\n", sysConf.volume / (DEF_CONF_SPKR_MAX / 100));
   conf_save();
   return CMD_NO_ERR;
}

static cmd_err_t set_brightness(_cl_param_t *sParam)
{
   uint8_t bright = 70;
   if (sParam->argc)
      bright = (uint8_t)strtol(sParam->argv[0], NULL, 10);
   else
   {
      tprintf("%d\n", sysConf.bright);
      return CMD_NO_ERR;
   }
   if (bright > 100)
      bright = 100;
   if (bright < 1)
      bright = 1;
   sysConf.bright = bright;
   lcd_set_bl(bright);
   conf_save();
   return CMD_NO_ERR;
}

static cmd_err_t set_font(_cl_param_t *sParam)
{
   uint8_t fontSelect = 0;
   if (!sParam->argc)
   {
      tprintf("Font selected: %d\n",sysConf.font);
   }
   else
   {
      fontSelect = (uint8_t)strtol(sParam->argv[0], NULL, 10);
      sysConf.font = set_system_font(fontSelect);
      conf_save();
   }
   return CMD_NO_ERR;
}

static cmd_err_t set_startup(_cl_param_t *sParam)
{
   if (sParam->argc)
   {
      tstrncpy(sysConf.startup, sParam->argv[0], CONFIG_STARTUP_LEN);
      conf_save();
   }
   tprintf("%s\n", *sysConf.startup ? sysConf.startup : "None");
   return CMD_NO_ERR;
}