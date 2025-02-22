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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "tstring.h"
#include "rshell.h"
#include "keyboard.h"
#include "sys_sercom.h"
#include "bcore.h"

static cmd_err_t mem_eer(_cl_param_t *sParam);
static cmd_err_t mem_eew(_cl_param_t *sParam);
static cmd_err_t mem_mem_free(_cl_param_t *sParam);
static cmd_err_t mem_malloc_test(_cl_param_t *sParam);
static bool iface_mem_init(bool verbose);

const _iface_t ifaceMemory =
{
    .name = "mem",
    .prompt = NULL,
    .desc = "Memory read/write",
    .init = iface_mem_init,
    .cmdList =
    {
        { .name = "eer", .desc = "eeprom read", .func = mem_eer },
        { .name = "eew", .desc = "eeprom write", .func = mem_eew },
        { .name = "mf", .desc = "show free memory", .func = mem_mem_free },
        { .name = "mt", .desc = "mem alloc test", .func = mem_malloc_test },
        { .name = NULL, .func = NULL },
    }
};

static bool iface_mem_init(bool verbose)
{
    static bool initialized = false;
    if (initialized) return true;
    initialized = true;
    if (verbose) tprintf("Interface \"mem\" initialized.\n");
    return true;
}

cmd_err_t mem_eer(_cl_param_t *sParam)
{
    uint16_t addr = sParam->argc ? (uint16_t)strtol(sParam->argv[0],NULL,0) : 0;
    uint16_t count = sParam->argc > 1 ? (uint16_t)strtol(sParam->argv[1],NULL,0) : 1;
    uint8_t lines=0;
    if (!count) count=1;
    static uint8_t data[8];
    while (count)
    {
        uint8_t chunk = (count >= 8) ? 8 : count % 8;
        sys_eeprom_read(addr,data,chunk);
        if (!dump8(addr,data,chunk,&lines)) return CMD_NO_ERR;
        addr += 8;
        count -= chunk;
    }
    return CMD_NO_ERR;
}

cmd_err_t mem_eew(_cl_param_t *sParam)
{
    if (sParam->argc < 2)
    {
        return CMD_MISSING_PARAM;
    }
    uint16_t addr = (uint16_t)strtol(sParam->argv[0],NULL,0);
    uint8_t size = (sParam->argc - 1) < 8 ? sParam->argc - 1 : 8;
    uint8_t data[8];
    uint8_t i;
    
    for (i=0; i<size; i++) data[i] = (uint8_t)strtol(sParam->argv[i + 1],NULL,0);
    if (!sys_eeprom_write(addr,data,size)) return "Failed to write to EEPROM.";
    else
    dump8(addr,data,size,&i);
    return CMD_NO_ERR;
}

cmd_err_t mem_mem_free(_cl_param_t *sParam)
{
    tprintf("\nTotal: %d\n Used: %d\n Free: %d\n\n",configTOTAL_HEAP_SIZE,configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize(),xPortGetFreeHeapSize());
    return CMD_NO_ERR;
}

uint8_t *mTestPtrs[10] = {0};
const uint8_t mtSizes[10] = {12,2,12,8,11,5,11,6,11,7};

static cmd_err_t mem_malloc_test(_cl_param_t *sParam)
{
    //tprintf("_bas_line_t size: %d\n",sizeof(_bas_line_t));
    size_t size = xPortGetFreeHeapSize();
     tprintf("Free: %d\n",xPortGetFreeHeapSize());
  for (uint8_t i=0;i<10;i++)
  {
      //if (mTestPtrs[i]) vPortFree(mTestPtrs[i]);
      mTestPtrs[i] = (uint8_t *)pvPortMalloc((size_t)(mtSizes[i]));
      tprintf("[%d]: Addr: 0x%8x, Size: %d\n",i,mTestPtrs[i],mtSizes[i]);
  }  
   tprintf("Used: %d, Free: %d\n",size-xPortGetFreeHeapSize(),xPortGetFreeHeapSize());
   return CMD_NO_ERR;
}