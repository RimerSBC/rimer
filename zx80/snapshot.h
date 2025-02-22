/**
 * Copyright (c) 2020 Sergey Sanders
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef SNAPSHOT_H_INCLUDED
#define SNAPSHOT_H_INCLUDED
/**

 Reference data from https://worldofspectrum.org/faq/reference/z80format.htm
Offset  Length  Description
        ---------------------------
        0       1       A register
        1       1       F register
        2       2       BC register pair (LSB, i.e. C, first)
        4       2       HL register pair
        6       2       Program counter
        8       2       Stack pointer
        10      1       Interrupt register
        11      1       Refresh register (Bit 7 is not significant!)
        12      1       Bit 0  : Bit 7 of the R-register
                        Bit 1-3: Border colour
                        Bit 4  : 1=Basic SamRom switched in
                        Bit 5  : 1=Block of data is compressed
                        Bit 6-7: No meaning
        13      2       DE register pair
        15      2       BC' register pair
        17      2       DE' register pair
        19      2       HL' register pair
        21      1       A' register
        22      1       F' register
        23      2       IY register (Again LSB first)
        25      2       IX register
        27      1       Interrupt flipflop, 0=DI, otherwise EI
        28      1       IFF2 (not particularly important...)
        29      1       Bit 0-1: Interrupt mode (0, 1 or 2)
                        Bit 2  : 1=Issue 2 emulation
                        Bit 3  : 1=Double interrupt frequency
                        Bit 4-5: 1=High video synchronisation
                                 3=Low video synchronisation
                                 0,2=Normal
                        Bit 6-7: 0=Cursor/Protek/AGF joystick
                                 1=Kempston joystick
                                 2=Sinclair 2 Left joystick (or user
                                   defined, for version 3 .z80 files)
                                 3=Sinclair 2 Right joystick
 */
 /**
  Version 2 and 3
  Offset  Length  Description
        ---------------------------
      * 30      2       Length of additional header block (see below)
      * 32      2       Program counter
      * 34      1       Hardware mode (see below)
      * 35      1       If in SamRam mode, bitwise state of 74ls259.
                        For example, bit 6=1 after an OUT 31,13 (=2*6+1)
                        If in 128 mode, contains last OUT to 0x7ffd
			If in Timex mode, contains last OUT to 0xf4
      * 36      1       Contains 0xff if Interface I rom paged
			If in Timex mode, contains last OUT to 0xff
      * 37      1       Bit 0: 1 if R register emulation on
                        Bit 1: 1 if LDIR emulation on
			Bit 2: AY sound in use, even on 48K machines
			Bit 6: (if bit 2 set) Fuller Audio Box emulation
			Bit 7: Modify hardware (see below)
      * 38      1       Last OUT to port 0xfffd (soundchip register number)
      * 39      16      Contents of the sound chip registers
        55      2       Low T state counter
        57      1       Hi T state counter
        58      1       Flag byte used by Spectator (QL spec. emulator)
                        Ignored by Z80 when loading, zero when saving
        59      1       0xff if MGT Rom paged
        60      1       0xff if Multiface Rom paged. Should always be 0.
        61      1       0xff if 0-8191 is ROM, 0 if RAM
        62      1       0xff if 8192-16383 is ROM, 0 if RAM
        63      10      5 x keyboard mappings for user defined joystick
        73      10      5 x ASCII word: keys corresponding to mappings above
        83      1       MGT type: 0=Disciple+Epson,1=Disciple+HP,16=Plus D
        84      1       Disciple inhibit button status: 0=out, 0ff=in
        85      1       Disciple inhibit flag: 0=rom pageable, 0ff=not
     ** 86      1       Last OUT to port 0x1ffd
*/
typedef struct
{
    uint8_t    A;
    uint8_t    F;
    uint8_t    C;
    uint8_t    B;
    uint8_t    L;
    uint8_t    H;
    uint8_t    PCL;
    uint8_t    PCH;
    uint8_t    SPL;
    uint8_t    SPH;
    uint8_t    I;
    uint8_t    R;
    uint8_t    hwCtrl;
    uint8_t    E;
    uint8_t    D;
    uint8_t    C_;
    uint8_t    B_;
    uint8_t    E_;
    uint8_t    D_;
    uint8_t    L_;
    uint8_t    H_;
    uint8_t    A_;
    uint8_t    F_;
    uint8_t    IYL;
    uint8_t    IYH;
    uint8_t    IXL;
    uint8_t    IXH;
    uint8_t    IE;
    uint8_t    IFF2;
    uint8_t    flags; 
} _snap_z80_hdr_t;
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
typedef struct
{
    uint8_t    I;
    uint8_t    L_;
    uint8_t    H_;
    uint8_t    E_;
    uint8_t    D_;
    uint8_t    C_;
    uint8_t    B_;
    uint8_t    F_;
    uint8_t    A_;
    uint8_t    L;
    uint8_t    H;
    uint8_t    E;
    uint8_t    D;
    uint8_t    C;
    uint8_t    B;
    uint8_t    IYL;
    uint8_t    IYH;
    uint8_t    IXL;
    uint8_t    IXH;
    
    uint8_t    IFF;
    uint8_t    R;
    uint8_t    F;
    uint8_t    A;
    uint8_t    SPL;
    uint8_t    SPH;
    uint8_t    IM;
    uint8_t    border;
    uint8_t    data[49152];
} _snap_sna_hdr_t;   
#define SNAPSHOT_HEADER_BLOCK_SIZE_POS    30
#define SNAPSHOT_V23_PC_POS               32

void load_snapshot_z80(uint8_t *data);
void load_snapshot_sna(uint8_t *data);
#endif //SNAPSHOT_H_INCLUDED