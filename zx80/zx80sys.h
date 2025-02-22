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

#ifndef Z80SYS_H_INCLUDED
#define Z80SYS_H_INCLUDED

#define Z80SYS_MEMORY_SIZE	(64 * 1024)// 64 Kbytes
#define Z80SYS_IOMEM_SIZE	(256)// bytes

#define Z80_CATCH_HALT	0
#define Z80_STATUS_FLAG_HALT 1

#define WII_ADDRESS 0x00a4

#include "z80cpu.h"
#define CLEAR_Z80_INT_FLAGS() tmrZX50Hz->INTFLAG.reg = tmrZX50Hz->INTFLAG.reg
#define  Z80Interrupt TC1_Handler

/// Internal flash partition
#define SNAPS_FLASH_SIZE    0x00060000
#define SNAP_SIZE           0x0000C000
#define ROM_SIZE            0x00004000
#define ROM_OFFSET          0x00020000
#define SNAPS_OFFSET        (ROM_OFFSET + ROM_SIZE)
#define ROM_ADDR            ((uint8_t *)ROM_OFFSET)
#define SNAPS_VOLUME        ((SNAPS_FLASH_SIZE-ROM_SIZE) / SNAP_SIZE)
typedef struct 
{
   uint8_t snap[SNAPS_VOLUME][SNAP_SIZE];
} _flash_snaps_partition_t;

enum
{
    SH_ZXCV,
    ASDFG,
    QWERT,
    _12345,
    _09876,
    POIUY,
    EN_LKJH,
    SP_SSP_MNB
};

uint8_t z80sys_input(uint16_t port);
void z80sys_output(uint16_t port,uint8_t data);
void zx80_task(void *vParam);

extern volatile bool zx50HzSignal;
extern volatile uint16_t addrMatch;
extern _flash_snaps_partition_t *snapStorage;
extern uint8_t *z80mem;//[Z80SYS_MEMORY_SIZE];
extern uint8_t borderRGB;
extern Z80_STATE z80state;
extern uint8_t keyRows[8];
extern TcCount16 *tmrZX50Hz;
void int50Hz_init(void);
void int50Hz_start(void);
void int50Hz_stop(void);
#endif //Z80SYS_H_INCLUDED