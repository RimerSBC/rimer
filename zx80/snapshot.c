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

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "string.h"
#include "tstring.h"
#include "bsp.h"
//#include "cmd.h"
#include "zxscreen.h"
#include "z80cpu.h"
#include "zx80sys.h"
#include "snapshot.h"

void snap_decode_block(uint8_t *dest,uint8_t *src,uint16_t blkSize)
{
    for (uint16_t i=0; i<blkSize && dest; i++)
    {
        if ((src[0] == 0xed) && (src[1] == 0xed))
        {
            src+=2;
            uint16_t blockCnt = *src++;
            uint8_t blockData = *src++;
            i+=blockCnt-1;
            while (blockCnt--)
            {
                *dest++ = blockData;
                if (dest == 0x0000) 
                    return; // something went wrong
            }
        }
        else
            *dest++ = *src++;
    }
}

void load_snapshot_z80(uint8_t *data)
{
    _snap_z80_hdr_t *snap = (_snap_z80_hdr_t *)data;
    bool ver2_3 = false;
//    tprintf("Load .z80 snap.\n");
    int50Hz_stop();
    z80cpu_stop();
    z80state.registers.word[Z80_AF] = snap->F + (snap->A << 8);
    z80state.registers.word[Z80_BC] = snap->C + (snap->B << 8);
    z80state.registers.word[Z80_HL] = snap->L + (snap->H << 8);
    z80state.registers.word[Z80_DE] = snap->E + (snap->D << 8);
    z80state.registers.word[Z80_SP] = snap->SPL + (snap->SPH << 8);
    z80state.registers.word[Z80_IY] = snap->IYL + (snap->IYH << 8);
    z80state.registers.word[Z80_IX] = snap->IXL + (snap->IXH << 8);
    z80state.alternates[0] = snap->F_ + (snap->A_ << 8);
    z80state.alternates[1] = snap->C_ + (snap->B_ << 8);
    z80state.alternates[2] = snap->E_ + (snap->D_ << 8);
    z80state.alternates[3] = snap->L_ + (snap->H_ << 8);
    z80state.pc = snap->PCL + (snap->PCH << 8);
    if (!z80state.pc)
    {
        z80state.pc = *(uint16_t *)&data[SNAPSHOT_V23_PC_POS];
        ver2_3 = true;
    }
    z80state.i = snap->I;
    z80state.r = snap->R;
    /// Hardware control
    z80state.r = (snap->hwCtrl & 0x01) ? z80state.r | 0x80 : z80state.r & ~0x80;
    borderRGB = ZxColour[0][(snap->hwCtrl) >> 1 & 0x07];
    z80state.iff1 = snap->IE;
    z80state.iff2 = snap->IFF2;
    z80state.im = snap->flags & 0x03;

    uint8_t *dataPtr = (uint8_t *)(data + sizeof(_snap_z80_hdr_t));
    if (ver2_3)
    {
        dataPtr += 2 + *(uint16_t *)&data[SNAPSHOT_HEADER_BLOCK_SIZE_POS] + (data[SNAPSHOT_HEADER_BLOCK_SIZE_POS] == 55 ? 1 : 0); // offset 2 bytes for the bytes 30 and 31 
        for (uint8_t blk=0; blk<3; blk++)
        {
            uint16_t blkSize = *(uint16_t *)dataPtr;
            uint16_t z80memOffset;
            switch(dataPtr[2]) // set block page
            {
            case 4:
                z80memOffset = 0x8000;
                break;
            case 5:
                z80memOffset = 0xc000;
                break;
            default:
            case 8:
                z80memOffset = 0x4000;
                break;
            }
            dataPtr += 3;
            if (blkSize == 0xffff)// 16384 uncompressed bytes
            {
                memcpy(&z80mem[z80memOffset],dataPtr,0x4000);
                dataPtr += 0x4000;
            }
            else
            {
                snap_decode_block(&z80mem[z80memOffset],dataPtr,0x4000); // decode 16Kb
                dataPtr += blkSize;
            }
        }
    }
    else // .z80 version 1
        snap_decode_block(&z80mem[ROM_SIZE],dataPtr,0xc000); // decode 48Kb

 //   print_debug_status();
}

/**
When the registers have been loaded, a RETN command is required to start the program. IFF2 is short for interrupt flip-flop 2, and for all practical purposes is the interrupt-enabled flag. Set means enabled.
   Offset   Size   Description
   ------------------------------------------------------------------------
   0        1      byte   I
   1        8      word   HL',DE',BC',AF'
   9        10     word   HL,DE,BC,IY,IX
   19       1      byte   Interrupt (bit 2 contains IFF2, 1=EI/0=DI)
   20       1      byte   R
   21       4      words  AF,SP
   25       1      byte   IntMode (0=IM0/1=IM1/2=IM2)
   26       1      byte   BorderColor (0..7, not used by Spectrum 1.7)
   27       49152  bytes  RAM dump 16384..65535
   ------------------------------------------------------------------------
   */
//void load_snapshot_sna(uint8_t *data)
void load_snapshot_sna(uint8_t *snapPtr)
{
    _snap_sna_hdr_t *snap = (_snap_sna_hdr_t *)snapPtr;
    tprintf("Load .sna snap.\n");
    int50Hz_stop();
    z80cpu_stop();
    z80state.registers.word[Z80_AF] = snap->F + (snap->A << 8);
    z80state.registers.word[Z80_BC] = snap->C + (snap->B << 8);
    z80state.registers.word[Z80_HL] = snap->L + (snap->H << 8);
    z80state.registers.word[Z80_DE] = snap->E + (snap->D << 8);
    z80state.registers.word[Z80_SP] = snap->SPL + (snap->SPH << 8);
    z80state.registers.word[Z80_IY] = snap->IYL + (snap->IYH << 8);
    z80state.registers.word[Z80_IX] = snap->IXL + (snap->IXH << 8);
    z80state.alternates[0] = snap->F_ + (snap->A_ << 8);
    z80state.alternates[1] = snap->C_ + (snap->B_ << 8);
    z80state.alternates[2] = snap->E_ + (snap->D_ << 8);
    z80state.alternates[3] = snap->L_ + (snap->H_ << 8);
    z80state.i = snap->I;
    z80state.r = snap->R;
    borderRGB = ZxColour[0][snap->border];
    z80state.iff1 = snap->IFF & 0x01;
    z80state.iff2 = (snap->IFF >> 1) & 0x01;
    z80state.im = snap->IM;
    memcpy(&z80mem[ROM_SIZE],snap->data,sizeof(snap->data));
//    print_debug_status();
    z80state.pc = z80mem[z80state.registers.word[Z80_SP]]; // implement "RETN"
    z80state.registers.word[Z80_SP] += 2;
//    print_debug_status();
    z80cpu_run();
    int50Hz_start();
}
