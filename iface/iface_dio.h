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
#ifndef _IFACE_DIO_H_INCLUDED
#define _IFACE_DIO_H_INCLUDED

#include "rshell.h"
#define IO_CONF_FILE_NAME ".io.conf"


typedef struct __attribute__((packed))
{
   uint32_t baud[3];
   uint8_t mux[2];
   uint8_t dir[2];
   uint8_t pin[2];
   uint8_t pull[2];
   uint8_t mode[3];
   uint8_t dioChan : 2;
   uint8_t sioChan : 2;
   uint8_t power : 3;
   uint8_t : 1; // reserved   
   uint8_t checkSum;
} _ioconf_t;

extern const _iface_t ifaceDIO;
extern _ioconf_t ioConf;
bool set_io_power(uint8_t chan, bool stat);

#endif //_IFACE_DIO_H_INCLUDED