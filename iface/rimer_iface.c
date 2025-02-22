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
* @file rimer_iface.c
* @author Sergey Sanders
* @date April 23
* @brief Register system interfaces 
*
*/

#include "rshell.h"
#include "iface_bas.h"
#include "iface_eeprom.h"
#include "iface_sd.h"
#include "iface_mem.h"
#include "iface_set.h"
#include "iface_dio.h"
#include "iface_aio.h"
#include "iface_sio.h"
#include "iface_zx80.h"

void interface_registstration(void)
{
    add_interface(&ifaceSetup);
    add_interface(&ifaceAIO);
    add_interface(&ifaceDIO);
    add_interface(&ifaceSIO);
    add_interface(&ifaceEEPROM);
    add_interface(&ifaceSD);
    add_interface(&ifaceBasic);
    add_interface(&ifaceMemory);
    add_interface(&ifaceZX80);
}