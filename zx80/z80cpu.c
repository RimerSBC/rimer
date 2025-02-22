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
 * based on Z80 processor emulator by Lin Ke-Fong
 * ---------------------------------------------------------------------------*/

#include "bsp.h"
#include "z80cpu.h"
#include "z80user.h"
#include "z80macros.h"
#include "zx80sys.h"
#include "zxscreen.h"
#include "string.h"
/* Indirect (HL) or prefixed indexed (IX + d) and (IY + d) memory operands are
 * encoded using the 3 bits "110" (0x06).
 */
#define INDIRECT_HL 0x06
/* Condition codes are encoded using 2 or 3 bits.  The xor table is needed for
 * negated conditions, it is used along with the and table.
 */
static uint8_t XOR_CONDITION_TABLE[8] =
    {
        Z80_Z_FLAG,
        0,
        Z80_C_FLAG,
        0,
        Z80_P_FLAG,
        0,
        Z80_S_FLAG,
        0,
};
static uint8_t AND_CONDITION_TABLE[8] =
    {
        Z80_Z_FLAG,
        Z80_Z_FLAG,
        Z80_C_FLAG,
        Z80_C_FLAG,
        Z80_P_FLAG,
        Z80_P_FLAG,
        Z80_S_FLAG,
        Z80_S_FLAG,
};
/* RST instruction restart addresses, encoded by Y() bits of the opcode. */
static uint8_t RST_TABLE[8] =
    {
        0x00,
        0x08,
        0x10,
        0x18,
        0x20,
        0x28,
        0x30,
        0x38,
};
/* There is an overflow if the xor of the carry out and the carry of the most
 * significant bit is not zero.
 */
static uint8_t OVERFLOW_TABLE[4] =
    {
        0,
        Z80_V_FLAG,
        Z80_V_FLAG,
        0,
};

static const uint8_t DefaultTStates[256] =
    {
        /**	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 		*/
        4, 10, 7, 6, 4, 4, 7, 4, 4, 11, 7, 6, 4, 4, 7, 4,          // 00
        8, 10, 7, 6, 4, 4, 7, 4, 12, 11, 7, 6, 4, 4, 7, 4,         // 10 djnz - OK
        7, 10, 16, 6, 4, 4, 7, 4, 7, 10, 16, 6, 4, 4, 7, 4,        // 20 jr nz/z - OK
        7, 10, 13, 6, 11, 11, 10, 4, 7, 11, 13, 6, 4, 4, 7, 4,     // 30 jr nc/c - OK
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 40
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 50
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 60
        7, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 70
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 80
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // 90
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // a0
        4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,            // b0
        5, 10, 10, 10, 10, 11, 7, 11, 5, 10, 10, 8, 10, 17, 7, 11, // c0 ret nz, call nz, call z - OK, CB group - OK
        5, 10, 10, 11, 10, 11, 7, 11, 5, 4, 10, 11, 10, 8, 7, 11,  // d0 ret nc, call nc, ret c - OK, DD group
        5, 10, 10, 19, 10, 11, 7, 11, 5, 4, 10, 4, 10, 8, 7, 11,   // e0 ret po, call po, ret pe, call pe - OK, ED group
        5, 10, 10, 4, 10, 11, 7, 11, 5, 6, 10, 4, 10, 8, 7, 11,    // f0 ret p, call p, ret m, call m - OK, FD group
};
static const uint8_t DefaultTStatesDDFD[256] =
    {
        /**	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 		*/
        8, 8, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8,        // 00
        8, 8, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8,        // 10
        8, 14, 20, 10, 8, 8, 11, 8, 8, 15, 20, 10, 8, 8, 11, 8, // 20
        8, 8, 8, 8, 23, 23, 19, 8, 8, 15, 8, 8, 8, 8, 8, 8,     // 30
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 40
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 50
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 60
        19, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 70
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 80
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // 90
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // a0
        8, 8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8,       // b0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,         // c0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,         // d0
        8, 14, 8, 23, 8, 15, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      // e0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 8, 8, 8, 8, 8, 8         // f0

};
#if 0
static const uint8_t DefaultTStatesED[256] =
    {
        /**	0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 		*/
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 00
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 10
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 20
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 30
       12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,     // 40
       12,12,15,20, 8, 8, 8, 9,12,12,15,20, 8, 8, 8, 9,     // 50
       12,12,15, 8, 8, 8, 8,18,12,12,15, 8, 8, 8, 8,18,     // 60
       12, 8,15,20, 8, 8, 8, 8,18,12,15,20, 8, 8, 8, 8,     // 70
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 80
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // 90
       16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,     // a0
       16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,     // b0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // c0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // d0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,     // e0
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8      // f0
};
#endif

Z80_STATE z80state;
uint16_t WS_Div_Table[24];
uint16_t TStatesTable[256];
uint16_t TStatesTableDDFD[256];
// uint16_t TStatesTableED[256];

/* Register decoding tables. */
void *register_table[16];
void *dd_register_table[16];
void *fd_register_table[16];

// #define PROFILE_PIN	PORT_PB04
#define Z80_MAX_CLOCK 4000000
#define Z80_MIN_CLOCK (SYS_CLOCK_FREQ / (60000 / 23)) // maximum cycles number is 23
#define Z80_DEFAULT_CLOCK 3500000
TcCount16 *tmrZ80Cpu = (TcCount16 *)TC0;
uint8_t dataOnBus = 0; // data to be used with the interrupts
uint16_t clkZ80div;
volatile uint8_t addWaitStates = 0;
void Z80Reset(Z80_STATE *state)
{
   int i;
   z80state.status = 0;
   AF = 0xffff;
   SP = 0xffff;
   z80state.i = z80state.pc = z80state.iff1 = z80state.iff2 = 0;
   z80state.im = Z80_INTERRUPT_MODE_0;
   /* Build register decoding tables for both 3-bit encoded 8-bit
    * registers and 2-bit encoded 16-bit registers. When an opcode is
    * prefixed by 0xdd, HL is replaced by IX. When 0xfd prefixed, HL is
    * replaced by IY.
    */

   /* 8-bit "R" registers. */
   register_table[0] = &z80state.registers.byte[Z80_B];
   register_table[1] = &z80state.registers.byte[Z80_C];
   register_table[2] = &z80state.registers.byte[Z80_D];
   register_table[3] = &z80state.registers.byte[Z80_E];
   register_table[4] = &z80state.registers.byte[Z80_H];
   register_table[5] = &z80state.registers.byte[Z80_L];
   /* Encoding 0x06 is used for indexed memory operands and direct HL or
    * IX/IY register access.
    */
   register_table[6] = &z80state.registers.word[Z80_HL];
   register_table[7] = &z80state.registers.byte[Z80_A];
   /* "Regular" 16-bit "RR" registers. */
   register_table[8] = &z80state.registers.word[Z80_BC];
   register_table[9] = &z80state.registers.word[Z80_DE];
   register_table[10] = &z80state.registers.word[Z80_HL];
   register_table[11] = &z80state.registers.word[Z80_SP];
   /* 16-bit "SS" registers for PUSH and POP instructions (note that SP is
    * replaced by AF).
    */
   register_table[12] = &z80state.registers.word[Z80_BC];
   register_table[13] = &z80state.registers.word[Z80_DE];
   register_table[14] = &z80state.registers.word[Z80_HL];
   register_table[15] = &z80state.registers.word[Z80_AF];
   /* 0xdd and 0xfd prefixed register decoding tables. */
   for (i = 0; i < 16; i++)
      dd_register_table[i] = fd_register_table[i] = register_table[i];
   dd_register_table[4] = &z80state.registers.byte[Z80_IXH];
   dd_register_table[5] = &z80state.registers.byte[Z80_IXL];
   dd_register_table[6] = &z80state.registers.word[Z80_IX];
   dd_register_table[10] = &z80state.registers.word[Z80_IX];
   dd_register_table[14] = &z80state.registers.word[Z80_IX];
   fd_register_table[4] = &z80state.registers.byte[Z80_IYH];
   fd_register_table[5] = &z80state.registers.byte[Z80_IYL];
   fd_register_table[6] = &z80state.registers.word[Z80_IY];
   fd_register_table[10] = &z80state.registers.word[Z80_IY];
   fd_register_table[14] = &z80state.registers.word[Z80_IY];
}

void Z80NonMaskableInterrupt(void)
{
   z80state.status = 0;
   z80state.iff2 = z80state.iff1;
   z80state.iff1 = 0;
   z80state.r = (z80state.r & 0x80) | ((z80state.r + 1) & 0x7f);
   SP -= 2;
   Z80_WRITE_WORD_INTERRUPT(SP, z80state.pc);
   z80state.pc = 0x0066;
   // return elapsed_cycles + 11;
}

void z80_set_clock(uint32_t fClkHz)
{
   uint16_t i;
   if (fClkHz < Z80_MIN_CLOCK)
      fClkHz = Z80_MIN_CLOCK;
   if (fClkHz > Z80_MAX_CLOCK)
      fClkHz = Z80_MAX_CLOCK;
   clkZ80div = SYS_CLOCK_FREQ / 2 / fClkHz; // 1 machine state @ 60MHz
   for (i = 0; i < 24; i++)
      WS_Div_Table[i] = clkZ80div * i;
   for (i = 0; i < 256; i++)
      TStatesTable[i] = clkZ80div * DefaultTStates[i];
   for (i = 0; i < 256; i++)
      TStatesTableDDFD[i] = clkZ80div * DefaultTStatesDDFD[i];
   //   for (i = 0; i < 256; i++)
   //      TStatesTableED[i] = clkZ80div * DefaultTStatesED[i];
}
uint32_t z80_get_clock(void)
{
   return SYS_CLOCK_FREQ / clkZ80div;
}
void z80cpu_run(void)
{
   vTaskResume(xLcdZxTask);
   tmrZ80Cpu->COUNT.reg = 0;
   tmrZ80Cpu->CC[0].reg = clkZ80div * 4; // Match comparator
   tmrZ80Cpu->CTRLBSET.bit.CMD = 0x01;   // start the timer
   tmrZX50Hz->CTRLBSET.bit.CMD = 0x01;   // start the timer
}
void z80cpu_stop(void)
{
   vTaskSuspend(xLcdZxTask);
   tmrZ80Cpu->CTRLBSET.bit.CMD = 0x02; // stop the timer
   tmrZX50Hz->CTRLBSET.bit.CMD = 0x02; // stop the timer
}

void z80_init(void)
{
   z80_set_clock(Z80_DEFAULT_CLOCK);
   Z80Reset(&z80state);
   /// z80CPU timer initialization
   REG_MCLK_APBAMASK |= MCLK_APBAMASK_TC0;            // enable TC0 clock
   REG_GCLK_PCHCTRL9 = CLK_60MHZ | GCLK_PCHCTRL_CHEN; // GCLK peripheral TC0 clock @ 12MHz
   tmrZ80Cpu->CC[0].reg = clkZ80div * 4;              // Match comparator value
   tmrZ80Cpu->INTENSET.bit.OVF = 1;                   // enable interrupt
   tmrZ80Cpu->WAVE.reg = 0x01;                        // Match compare
   tmrZ80Cpu->CTRLA.bit.ENABLE = 1;
   // while(!tmrZ80Cpu->SYNCBUSY.bit.ENABLE)
   __asm("nop");
   vTaskDelay(1);
   addWaitStates = WS_Div_Table[3];
   tmrZ80Cpu->CTRLBSET.bit.CMD = 0x02; // stop the timer
   vTaskSuspend(xLcdZxTask);
   NVIC_EnableIRQ(TC0_IRQn);
   NVIC_SetPriority(TC0_IRQn, 0);
}

#define R_REG_CNT ((uint8_t)SysTick->VAL & 0x7f)
#define TSTATES_ADD(n) tmrZ80Cpu->CC[0].reg += WS_Div_Table[n]
// #define TSTATES_ADD(n)

#if 1
void __attribute__((long_call, section(".ramfunc"), optimize("3"))) TC0_Handler(void)
#else
#warning Z80cpu compiled in debug mode!
void __attribute__((long_call, section(".ramfunc"), optimize("0"))) TC0_Handler(void)
#endif
{
#include "tables.h"
   void **registers;
   uint8_t opcode;                                  //,instruction;
   tmrZ80Cpu->INTFLAG.reg = tmrZ80Cpu->INTFLAG.reg; // clear interrupt flag
   if (addrMatch == z80state.pc)
   {
      if (addrMatch)
      {
         tmrZ80Cpu->CTRLBSET.bit.CMD = 0x02; // stop the timer
         tmrZX50Hz->CTRLBSET.bit.CMD = 0x02; // stop the timer
         addrMatch = 0;
         return;
      }
   }
   opcode = Z80_FETCH_BYTE(z80state.pc++);
   tmrZ80Cpu->CC[0].reg = TStatesTable[opcode];
   registers = register_table;
   goto *INSTRUCTION_TABLE[opcode];
#define exec_done_wt()                                  \
   tmrZ80Cpu->CC[0].reg += WS_Div_Table[addWaitStates]; \
   addWaitStates = 0;                                   \
   return;
// #define exec_done_wt() return;
#define exec_done() return;
   /* 8-bit load group. */
LD_R_R:
{
   R(Y(opcode)) = R(Z(opcode));
   exec_done();
}
LD_R_N:
{
   READ_N(R(Y(opcode)));
   exec_done();
}
LD_R_INDIRECT_HL:
{
   if (registers == register_table)
   {
      R(Y(opcode)) = READ_BYTE(HL);
   }
   else
   {
      uint16_t addr;
      int8_t dd;
      READ_D(dd);
      addr = (int)HL_IX_IY + dd;
      S(Y(opcode)) = READ_BYTE(addr);
   }
   exec_done();
}
LD_INDIRECT_HL_R:
{
   if (registers == register_table)
   {
      WRITE_BYTE(HL, R(Z(opcode)));
   }
   else
   {
      uint16_t addr;
      int8_t dd;
      READ_D(dd);
      addr = (int)HL_IX_IY + dd;
      WRITE_BYTE(addr, S(Z(opcode)));
   }
   exec_done();
}
LD_INDIRECT_HL_N:
{
   int n;
   if (registers == register_table)
   {
      READ_N(n);
      WRITE_BYTE(HL, n);
   }
   else
   {
      uint16_t addr;
      int8_t dd;
      READ_D(dd);
      addr = (int)HL_IX_IY + dd;
      READ_N(n);
      WRITE_BYTE(addr, n);
   }
   exec_done();
}
LD_A_INDIRECT_BC:
{
   A = READ_BYTE(BC);
   exec_done();
}
LD_A_INDIRECT_DE:
{
   A = READ_BYTE(DE);
   exec_done();
}
LD_A_INDIRECT_NN:
{
   int nn;
   READ_NN(nn);
   A = READ_BYTE(nn);
   exec_done();
}
LD_INDIRECT_BC_A:
{
   WRITE_BYTE(BC, A);
   exec_done();
}
LD_INDIRECT_DE_A:
{
   WRITE_BYTE(DE, A);
   exec_done();
}
LD_INDIRECT_NN_A:
{
   int nn;
   READ_NN(nn);
   WRITE_BYTE(nn, A);
   exec_done();
}
LD_A_I_LD_A_R:
{
   TSTATES_ADD(1);
   int a, f;
   a = opcode == OPCODE_LD_A_I ? z80state.i : (z80state.r & 0x80) | R_REG_CNT;
   f = SZYX_FLAGS_TABLE[a];
   /* Note: On a real processor, if an interrupt
    * occurs during the execution of either
    * "LD A, I" or "LD A, R", the parity flag is
    * reset. That can never happen here.
    */
   f |= z80state.iff2 << Z80_P_FLAG_SHIFT;
   f |= F & Z80_C_FLAG;
   AF = (a << 8) | f;
   exec_done();
}
LD_I_A_LD_R_A:
{
   TSTATES_ADD(1);
   if (opcode == OPCODE_LD_I_A)
      z80state.i = A;
   else
   {
      z80state.r = A;
      // r = A & 0x7f;
   }
   exec_done();
}
   /* 16-bit load group. */
LD_RR_NN:
{
   READ_NN(RR(P(opcode)));
   exec_done();
}
LD_HL_INDIRECT_NN:
{
   int nn;
   READ_NN(nn);
   HL_IX_IY = READ_WORD(nn);
   exec_done();
}
LD_RR_INDIRECT_NN:
{
   TSTATES_ADD(12);
   int nn;
   READ_NN(nn);
   RR(P(opcode)) = READ_WORD(nn);
   exec_done();
}
LD_INDIRECT_NN_HL:
{
   int nn;
   READ_NN(nn);
   WRITE_WORD(nn, HL_IX_IY);
   exec_done();
}
LD_INDIRECT_NN_RR:
{
   TSTATES_ADD(12);
   int nn;
   READ_NN(nn);
   WRITE_WORD(nn, RR(P(opcode)));
   exec_done();
}
LD_SP_HL:
{
   SP = HL_IX_IY;
   exec_done();
}
PUSH_SS:
{
   PUSH(SS(P(opcode)));
   exec_done();
}
POP_SS:
{
   POP(SS(P(opcode)));
   exec_done();
}
   /* Exchange, block transfer and search group. */
EX_DE_HL:
{
   EXCHANGE(DE, HL);
   exec_done();
}
EX_AF_AF_PRIME:
{
   EXCHANGE(AF, z80state.alternates[Z80_AF]);
   exec_done();
}
EXX:
{
   EXCHANGE(BC, z80state.alternates[Z80_BC]);
   EXCHANGE(DE, z80state.alternates[Z80_DE]);
   EXCHANGE(HL, z80state.alternates[Z80_HL]);
   exec_done();
}
EX_INDIRECT_SP_HL:
{
   int t;
   t = READ_WORD(SP);
   WRITE_WORD(SP, HL_IX_IY);
   HL_IX_IY = t;
   exec_done();
}
LDI_LDD:
{
   TSTATES_ADD(8);
   int n, f, d;
   n = READ_BYTE(HL);
   WRITE_BYTE(DE, n);
   f = F & SZC_FLAGS;
   f |= --BC ? Z80_P_FLAG : 0;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   n += A;
   f |= n & Z80_X_FLAG;
   f |= (n << (Z80_Y_FLAG_SHIFT - 1)) & Z80_Y_FLAG;
#endif
   F = f;
   d = opcode == OPCODE_LDI ? +1 : -1;
   DE += d;
   HL += d;
   exec_done();
}
LDIR_LDDR:
{
   int d, f, bc, de, hl, n;
   d = opcode == OPCODE_LDIR ? +1 : -1;
   f = F & SZC_FLAGS;
   bc = BC;
   de = DE;
   hl = HL;
   n = Z80_READ_BYTE(hl);
   Z80_WRITE_BYTE(de, n);
   if (opcode == OPCODE_LDIR)
   {
      HL++;
      DE++;
   }
   else
   {
      HL--;
      DE--;
   }
   hl += d;
   de += d;
   if (--bc)
   {
      TSTATES_ADD(13);
      z80state.pc -= 2;
   }
   else
   {
      TSTATES_ADD(8);
   }
   HL = hl;
   DE = de;
   BC = bc;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   n += A;
   f |= n & Z80_X_FLAG;
   f |= (n << (Z80_Y_FLAG_SHIFT - 1)) & Z80_Y_FLAG;
#endif
   F = f;
   exec_done();
}
CPI_CPD:
{
   TSTATES_ADD(8);
   int a, n, z, f;
   a = A;
   n = READ_BYTE(HL);
   z = a - n;
   HL += opcode == OPCODE_CPI ? +1 : -1;
   f = (a ^ n ^ z) & Z80_H_FLAG;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   n = z - (f >> Z80_H_FLAG_SHIFT);
   f |= (n << (Z80_Y_FLAG_SHIFT - 1)) & Z80_Y_FLAG;
   f |= n & Z80_X_FLAG;
#endif
   f |= SZYX_FLAGS_TABLE[z & 0xff] & SZ_FLAGS;
   f |= --BC ? Z80_P_FLAG : 0;
   F = f | Z80_N_FLAG | (F & Z80_C_FLAG);
   exec_done();
}
CPIR_CPDR:
{
   int d, a, bc, hl, n, z, f;
   d = opcode == OPCODE_CPIR ? +1 : -1;
   a = A;
   bc = BC;
   hl = HL;
   // r -= 2;
   // r += 2;
   n = Z80_READ_BYTE(hl);
   z = a - n;
   hl += d;
   if (--bc && z)
   {
      TSTATES_ADD(13);
      z80state.pc -= 2;
   }
   else
   {
      TSTATES_ADD(8);
   }
   HL = hl;
   BC = bc;
   f = (a ^ n ^ z) & Z80_H_FLAG;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   n = z - (f >> Z80_H_FLAG_SHIFT);
   f |= (n << (Z80_Y_FLAG_SHIFT - 1)) & Z80_Y_FLAG;
   f |= n & Z80_X_FLAG;
#endif
   f |= SZYX_FLAGS_TABLE[z & 0xff] & SZ_FLAGS;
   f |= bc ? Z80_P_FLAG : 0;
   F = f | Z80_N_FLAG | (F & Z80_C_FLAG);
   exec_done();
}
   /* 8-bit arithmetic and logical group. */
ADD_R:
{
   ADD(R(Z(opcode)));
   exec_done();
}
ADD_N:
{
   int n;
   READ_N(n);
   ADD(n);
   exec_done();
}
ADD_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   ADD(x);
   exec_done();
}
ADC_R:
{
   ADC(R(Z(opcode)));
   exec_done();
}
ADC_N:
{
   int n;
   READ_N(n);
   ADC(n);
   exec_done();
}
ADC_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   ADC(x);
   exec_done();
}
SUB_R:
{
   SUB(R(Z(opcode)));
   exec_done();
}
SUB_N:
{
   int n;
   READ_N(n);
   SUB(n);
   exec_done();
}
SUB_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   SUB(x);
   exec_done();
}
SBC_R:
{
   SBC(R(Z(opcode)));
   exec_done();
}
SBC_N:
{
   int n;
   READ_N(n);
   SBC(n);
   exec_done();
}
SBC_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   SBC(x);
   exec_done();
}
AND_R:
{
   AND(R(Z(opcode)));
   exec_done();
}
AND_N:
{
   int n;
   READ_N(n);
   AND(n);
   exec_done();
}
AND_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   AND(x);
   exec_done();
}
OR_R:
{
   OR(R(Z(opcode)));
   exec_done();
}
OR_N:
{
   int n;
   READ_N(n);
   OR(n);
   exec_done();
}
OR_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   OR(x);
   exec_done();
}
XOR_R:
{
   XOR(R(Z(opcode)));
   exec_done();
}
XOR_N:
{
   int n;
   READ_N(n);
   XOR(n);
   exec_done();
}
XOR_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   XOR(x);
   exec_done();
}
CP_R:
{
   CP(R(Z(opcode)));
   exec_done();
}
CP_N:
{
   int n;
   READ_N(n);
   CP(n);
   exec_done();
}
CP_INDIRECT_HL:
{
   int x;
   READ_INDIRECT_HL(x);
   CP(x);
   exec_done();
}
INC_R:
{
   INC(R(Y(opcode)));
   exec_done();
}
INC_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      x = READ_BYTE(HL);
      INC(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      uint16_t addr;
      int8_t dd;
      READ_D(dd);
      addr = (int)HL_IX_IY + dd;
      x = READ_BYTE(addr);
      INC(x);
      WRITE_BYTE(addr, x);
   }
   exec_done();
}
DEC_R:
{
   DEC(R(Y(opcode)));
   exec_done();
}
DEC_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      x = READ_BYTE(HL);
      DEC(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      uint16_t addr;
      int8_t dd;
      READ_D(dd);
      addr = (int)HL_IX_IY + dd;
      x = READ_BYTE(addr);
      DEC(x);
      WRITE_BYTE(addr, x);
   }
   exec_done();
}
   /* General-purpose arithmetic and CPU control group. */
DAA:
{
   int a, c, d;
   /* The following algorithm is from
    * comp.sys.sinclair's FAQ.
    */
   a = A;
   if (a > 0x99 || (F & Z80_C_FLAG))
   {
      c = Z80_C_FLAG;
      d = 0x60;
   }
   else
      c = d = 0;
   if ((a & 0x0f) > 0x09 || (F & Z80_H_FLAG))
      d += 0x06;
   A += F & Z80_N_FLAG ? -d : +d;
   F = SZYXP_FLAGS_TABLE[A] | ((A ^ a) & Z80_H_FLAG) | (F & Z80_N_FLAG) | c;
   exec_done();
}
CPL:
{
   A = ~A;
   F = (F & (SZPV_FLAGS | Z80_C_FLAG))
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (A & YX_FLAGS)
#endif
       | Z80_H_FLAG | Z80_N_FLAG;
   exec_done();
}
NEG:
{
   int a, f, z, c;
   a = A;
   z = -a;
   c = a ^ z;
   f = Z80_N_FLAG | (c & Z80_H_FLAG);
   f |= SZYX_FLAGS_TABLE[z &= 0xff];
   c &= 0x0180;
   f |= OVERFLOW_TABLE[c >> 7];
   f |= c >> (8 - Z80_C_FLAG_SHIFT);
   A = z;
   F = f;
   exec_done();
}
CCF:
{
   int c;
   c = F & Z80_C_FLAG;
   F = (F & SZPV_FLAGS) | (c << Z80_H_FLAG_SHIFT)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (A & YX_FLAGS)
#endif
       | (c ^ Z80_C_FLAG);
   exec_done();
}
SCF:
{
   F = (F & SZPV_FLAGS)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (A & YX_FLAGS)
#endif
       | Z80_C_FLAG;
   exec_done();
}
NOP:
{
   exec_done();
}
HALT:
{
#if Z80_CATCH_HALT
   z80state.status = Z80_STATUS_HALT;
   tmrZ80Cpu->CTRLBSET.bit.CMD = 0x02; // stop the CPU CLOCK fimer
#else
   /* If an HALT instruction is executed, the Z80
    * keeps executing NOPs until an interrupt is
    * generated. Basically nothing happens for the
    * remaining number of cycles.
    */
#endif
   // goto stop_emulation;
   exec_done();
}
DI:
{
   z80state.iff1 = z80state.iff2 = 0;
#ifdef Z80_CATCH_DI
   z80state.status = Z80_STATUS_FLAG_DI;
   goto stop_emulation;
#else
   /* No interrupt can be accepted right after
    * a DI or EI instruction on an actual Z80
    * processor. By adding 4 cycles to
    * number_cycles, at least one more
    * instruction will be executed. However, this
    * will fail if the next instruction has
    * multiple 0xdd or 0xfd prefixes and
    * Z80_PREFIX_FAILSAFE is defined, but that
    * is an unlikely pathological case.
    */
   exec_done();
#endif
}
EI:
{
   z80state.iff1 = z80state.iff2 = 1;
#ifdef Z80_CATCH_EI
   z80state.status = Z80_STATUS_FLAG_EI;
   goto stop_emulation;
#else
   /* See comment for DI. */
   exec_done();
#endif
}
IM_N:
{
   /* "IM 0/1" (0xed prefixed opcodes 0x4e and
    * 0x6e) is treated like a "IM 0".
    */
   if ((Y(opcode) & 0x03) <= 0x01)
      z80state.im = Z80_INTERRUPT_MODE_0;
   else if (!(Y(opcode) & 1))
      z80state.im = Z80_INTERRUPT_MODE_1;
   else
      z80state.im = Z80_INTERRUPT_MODE_2;
   exec_done();
}
   /* 16-bit arithmetic group. */
ADD_HL_RR:
{
   int x, y, z, f, c;
   x = HL_IX_IY;
   y = RR(P(opcode));
   z = x + y;
   c = x ^ y ^ z;
   f = F & SZPV_FLAGS;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   f |= (z >> 8) & YX_FLAGS;
   f |= (c >> 8) & Z80_H_FLAG;
#endif
   f |= c >> (16 - Z80_C_FLAG_SHIFT);
   HL_IX_IY = z;
   F = f;
   exec_done();
}
ADC_HL_RR:
{
   TSTATES_ADD(7);
   int x, y, z, f, c;
   x = HL;
   y = RR(P(opcode));
   z = x + y + (F & Z80_C_FLAG);
   c = x ^ y ^ z;
   f = z & 0xffff ? (z >> 8) & SYX_FLAGS : Z80_Z_FLAG;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   f |= (c >> 8) & Z80_H_FLAG;
#endif
   f |= OVERFLOW_TABLE[c >> 15];
   f |= z >> (16 - Z80_C_FLAG_SHIFT);
   HL = z;
   F = f;
   exec_done();
}
SBC_HL_RR:
{
   TSTATES_ADD(7);
   int x, y, z, f, c;
   x = HL;
   y = RR(P(opcode));
   z = x - y - (F & Z80_C_FLAG);
   c = x ^ y ^ z;
   f = Z80_N_FLAG;
   f |= z & 0xffff ? (z >> 8) & SYX_FLAGS : Z80_Z_FLAG;
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
   f |= (c >> 8) & Z80_H_FLAG;
#endif
   c &= 0x018000;
   f |= OVERFLOW_TABLE[c >> 15];
   f |= c >> (16 - Z80_C_FLAG_SHIFT);
   HL = z;
   F = f;
   exec_done();
}
INC_RR:
{
   int x;
   x = RR(P(opcode));
   x++;
   RR(P(opcode)) = x;
   exec_done();
}
DEC_RR:
{
   int x;
   x = RR(P(opcode));
   x--;
   RR(P(opcode)) = x;
   exec_done();
}
   /* Rotate and shift group. */
RLCA:
{
   A = (A << 1) | (A >> 7);
   F = (F & SZPV_FLAGS) | (A & (YX_FLAGS | Z80_C_FLAG));
   exec_done();
}
RLA:
{
   int a, f;
   a = A << 1;
   f = (F & SZPV_FLAGS)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (a & YX_FLAGS)
#endif
       | (A >> 7);
   A = a | (F & Z80_C_FLAG);
   F = f;
   exec_done();
}
RRCA:
{
   int c;
   c = A & 0x01;
   A = (A >> 1) | (A << 7);
   F = (F & SZPV_FLAGS)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (A & YX_FLAGS)
#endif
       | c;
   exec_done();
}
RRA:
{
   int c;
   c = A & 0x01;
   A = (A >> 1) | ((F & Z80_C_FLAG) << 7);
   F = (F & SZPV_FLAGS)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (A & YX_FLAGS)
#endif
       | c;
   exec_done();
}
RLC_R:
{
   RLC(R(Z(opcode)));
   exec_done();
}
RLC_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      RLC(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      RLC(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
RL_R:
{
   RL(R(Z(opcode)));
   exec_done();
}
RL_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      RL(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      RL(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
RRC_R:
{
   RRC(R(Z(opcode)));
   exec_done();
}
RRC_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      RRC(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      RRC(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
RR_R:
{
   RR_INSTRUCTION(R(Z(opcode)));
   exec_done();
}
RR_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      RR_INSTRUCTION(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      RR_INSTRUCTION(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
SLA_R:
{
   SLA(R(Z(opcode)));
   exec_done();
}
SLA_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      SLA(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      SLA(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
SLL_R:
{
   SLL(R(Z(opcode)));
   exec_done();
}
SLL_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      SLL(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      SLL(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
SRA_R:
{
   SRA(R(Z(opcode)));
   exec_done();
}
SRA_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      SRA(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      SRA(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
SRL_R:
{
   SRL(R(Z(opcode)));
   exec_done();
}
SRL_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      SRL(x);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      SRL(x);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
RLD_RRD:
{
   TSTATES_ADD(10);
   int x, y;
   x = READ_BYTE(HL);
   y = (A & 0xf0) << 8;
   y |= opcode == OPCODE_RLD ? (x << 4) | (A & 0x0f) : ((x & 0x0f) << 8) | ((A & 0x0f) << 4) | (x >> 4);
   WRITE_BYTE(HL, y);
   y >>= 8;
   A = y;
   F = SZYXP_FLAGS_TABLE[y] | (F & Z80_C_FLAG);
   exec_done();
}
   /* Bit set, reset, and test group. */
BIT_B_R:
{
   int x;
   x = R(Z(opcode)) & (1 << Y(opcode));
   F = (x ? 0 : Z80_Z_FLAG | Z80_P_FLAG)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (x & Z80_S_FLAG) | (R(Z(opcode)) & YX_FLAGS)
#endif
       | Z80_H_FLAG | (F & Z80_C_FLAG);
   exec_done();
}
BIT_B_INDIRECT_HL:
{
   int d, x;
   if (registers == register_table)
   {
      TSTATES_ADD(4);
      d = HL;
   }
   else
   {
      TSTATES_ADD(12);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      z80state.pc += 2;
   }
   x = READ_BYTE(d);
   x &= 1 << Y(opcode);
   F = (x ? 0 : Z80_Z_FLAG | Z80_P_FLAG)
#ifndef Z80_DOCUMENTED_FLAGS_ONLY
       | (x & Z80_S_FLAG) | (d & YX_FLAGS)
#endif
       | Z80_H_FLAG | (F & Z80_C_FLAG);
   exec_done();
}
SET_B_R:
{
   R(Z(opcode)) |= 1 << Y(opcode);
   exec_done();
}
SET_B_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      x |= 1 << Y(opcode);
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      x |= 1 << Y(opcode);
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
RES_B_R:
{
   R(Z(opcode)) &= ~(1 << Y(opcode));
   exec_done();
}
RES_B_INDIRECT_HL:
{
   int x;
   if (registers == register_table)
   {
      TSTATES_ADD(7);
      x = READ_BYTE(HL);
      x &= ~(1 << Y(opcode));
      WRITE_BYTE(HL, x);
   }
   else
   {
      int d;
      TSTATES_ADD(15);
      d = Z80_FETCH_BYTE(z80state.pc);
      d = ((signed char)d) + HL_IX_IY;
      x = READ_BYTE(d);
      x &= ~(1 << Y(opcode));
      WRITE_BYTE(d, x);
      if (Z(opcode) != INDIRECT_HL)
         R(Z(opcode)) = x;
      z80state.pc += 2;
   }
   exec_done();
}
   /* Jump group. */
JP_NN:
{
   z80state.pc = Z80_FETCH_WORD(z80state.pc);
   exec_done();
}
JP_CC_NN:
{
   if (CC(Y(opcode)))
   {
      z80state.pc = Z80_FETCH_WORD(z80state.pc);
   }
   else
   {
#ifdef Z80_FALSE_CONDITION_FETCH
      Z80_FETCH_WORD(z80state.pc);
#endif
      z80state.pc += 2;
   }
   exec_done();
}
JR_E:
{
   int e;
   e = Z80_FETCH_BYTE(z80state.pc);
   z80state.pc += ((signed char)e) + 1;
   exec_done();
}
JR_DD_E:
{
   int e;
   if (DD(Q(opcode)))
   {
      TSTATES_ADD(5);
      e = Z80_FETCH_BYTE(z80state.pc);
      z80state.pc += ((signed char)e) + 1;
   }
   else
   {
#ifdef Z80_FALSE_CONDITION_FETCH
      Z80_FETCH_BYTE(pc, e);
#endif
      z80state.pc++;
   }
   exec_done();
}
JP_HL:
{
   z80state.pc = HL_IX_IY;
   exec_done();
}
DJNZ_E:
{
   int e;
   if (--B)
   {
      TSTATES_ADD(5);
      e = Z80_FETCH_BYTE(z80state.pc);
      z80state.pc += ((signed char)e) + 1;
   }
   else
   {
#ifdef Z80_FALSE_CONDITION_FETCH
      Z80_FETCH_BYTE(pc, e);
#endif
      z80state.pc++;
   }
   exec_done();
}
   /* Call and return group. */
CALL_NN:
{
   int nn;
   READ_NN(nn);
   PUSH(z80state.pc);
   z80state.pc = nn;
   exec_done();
}
CALL_CC_NN:
{
   int nn;
   if (CC(Y(opcode)))
   {
      TSTATES_ADD(7);
      READ_NN(nn);
      PUSH(z80state.pc);
      z80state.pc = nn;
   }
   else
   {
#ifdef Z80_FALSE_CONDITION_FETCH
      Z80_FETCH_WORD(pc, nn);
#endif
      z80state.pc += 2;
   }
   exec_done();
}
RET:
{
   POP(z80state.pc);
   exec_done();
}
RET_CC:
{
   if (CC(Y(opcode)))
   {
      TSTATES_ADD(6);
      POP(z80state.pc);
   }
   exec_done();
}
RETI_RETN:
{
   TSTATES_ADD(6);
   z80state.iff1 = z80state.iff2;
   POP(z80state.pc);
#if defined(Z80_CATCH_RETI) && defined(Z80_CATCH_RETN)
   z80state.status = opcode == OPCODE_RETI ? Z80_STATUS_FLAG_RETI : Z80_STATUS_FLAG_RETN;
   goto stop_emulation;
#elif defined(Z80_CATCH_RETI)
   z80state.status = Z80_STATUS_FLAG_RETI;
   goto stop_emulation;
#elif defined(Z80_CATCH_RETN)
   z80state.status = Z80_STATUS_FLAG_RETN;
   goto stop_emulation;
#else
   exec_done();
#endif
}
RST_P:
{
   PUSH(z80state.pc);
   z80state.pc = RST_TABLE[Y(opcode)];
   exec_done();
}
   /* Input and output group. */
IN_A_N:
{
   uint8_t n;
   READ_N(n);
   A = Z80_INPUT_BYTE(((uint16_t)A << 8) + n);
   exec_done_wt();
}
IN_R_C:
{
   TSTATES_ADD(4);
   uint8_t x = Z80_INPUT_BYTE(((uint16_t)B << 8) + C);
   if (Y(opcode) != INDIRECT_HL)
      R(Y(opcode)) = x;
   F = SZYXP_FLAGS_TABLE[x] | (F & Z80_C_FLAG);
   exec_done_wt();
}
   /* Some of the undocumented flags for "INI", "IND",
    * "INIR", "INDR",  "OUTI", "OUTD", "OTIR", and
    * "OTDR" are really really strange. The emulator
    * implements the specifications described in "The
    * Undocumented Z80 Documented Version 0.91".
    */
INI_IND:
{
   TSTATES_ADD(8);
   uint8_t x, f;
   x = Z80_INPUT_BYTE(((uint16_t)B << 8) + C);
   WRITE_BYTE(HL, x);
   f = SZYX_FLAGS_TABLE[--B & 0xff] | (x >> (7 - Z80_N_FLAG_SHIFT));
   if (opcode == OPCODE_INI)
   {
      HL++;
      x += (C + 1) & 0xff;
   }
   else
   {
      HL--;
      x += (C - 1) & 0xff;
   }
   f |= x & 0x0100 ? HC_FLAGS : 0;
   f |= SZYXP_FLAGS_TABLE[(x & 0x07) ^ B] & Z80_P_FLAG;
   F = f;
   exec_done_wt();
}
INIR_INDR:
{
   int d, b, hl, x, f;
#ifdef Z80_HANDLE_SELF_MODIFYING_CODE
   int p, q;
   p = (pc - 2) & 0xffff;
   q = (pc - 1) & 0xffff;
#endif
   d = opcode == OPCODE_INIR ? +1 : -1;
   b = B;
   hl = HL;
   x = Z80_INPUT_BYTE(((uint16_t)B << 8) + C);
   Z80_WRITE_BYTE(hl, x);
   hl += d;
   if (--b)
   {
      TSTATES_ADD(13);
      z80state.pc -= 2;
      f = SZYX_FLAGS_TABLE[b];
   }
   else
   {
      f = Z80_Z_FLAG;
      TSTATES_ADD(8);
   }
#ifdef Z80_HANDLE_SELF_MODIFYING_CODE
   if (((hl - d) & 0xffff) == p || ((hl - d) & 0xffff) == q)
   {
      f = SZYX_FLAGS_TABLE[b];
      pc -= 2;
      exec_done();
   }
#endif
   HL = hl;
   B = b;
   f |= x >> (7 - Z80_N_FLAG_SHIFT);
   x += (C + d) & 0xff;
   f |= x & 0x0100 ? HC_FLAGS : 0;
   f |= SZYXP_FLAGS_TABLE[(x & 0x07) ^ b] & Z80_P_FLAG;
   F = f;
   exec_done_wt();
}
OUT_N_A:
{
   int n;
   READ_N(n);
   Z80_OUTPUT_BYTE(((uint16_t)A << 8) + n, A);
   exec_done_wt();
}
OUT_C_R:
{
   TSTATES_ADD(4);
   int x;
   x = Y(opcode) != INDIRECT_HL ? R(Y(opcode)) : 0;
   Z80_OUTPUT_BYTE(((uint16_t)B << 8) + C, x);
   exec_done_wt();
}
OUTI_OUTD:
{
   TSTATES_ADD(8);
   int x, f;
   x = READ_BYTE(HL);
   Z80_OUTPUT_BYTE(((uint16_t)B << 8) + C, x);
   HL += opcode == OPCODE_OUTI ? +1 : -1;
   f = SZYX_FLAGS_TABLE[--B & 0xff] | (x >> (7 - Z80_N_FLAG_SHIFT));
   x += HL & 0xff;
   f |= x & 0x0100 ? HC_FLAGS : 0;
   f |= SZYXP_FLAGS_TABLE[(x & 0x07) ^ B] & Z80_P_FLAG;
   F = f;
   exec_done_wt();
}
OTIR_OTDR:
{
   int d, b, hl, x, f;
   d = opcode == OPCODE_OTIR ? +1 : -1;
   b = B;
   hl = HL;
   x = Z80_READ_BYTE(hl);
   Z80_OUTPUT_BYTE(((uint16_t)B << 8) + C, x);
   hl += d;
   if (--b)
   {
      TSTATES_ADD(13);
      z80state.pc -= 2;
      f = SZYX_FLAGS_TABLE[b];
   }
   else
   {
      f = Z80_Z_FLAG;
      TSTATES_ADD(8);
   }
   HL = hl;
   B = b;
   f |= x >> (7 - Z80_N_FLAG_SHIFT);
   x += hl & 0xff;
   f |= x & 0x0100 ? HC_FLAGS : 0;
   f |= SZYXP_FLAGS_TABLE[(x & 0x07) ^ b] & Z80_P_FLAG;
   F = f;
   exec_done_wt();
}
   /* Prefix group. */
CB_PREFIX:
{
   /* Special handling if the 0xcb prefix is
    * prefixed by a 0xdd or 0xfd prefix.
    */
   if (registers != register_table)
   {

      /* Indexed memory access routine will
       * correctly update pc.
       */
      opcode = Z80_FETCH_BYTE(z80state.pc + 1);
   }
   else
   {
      opcode = Z80_FETCH_BYTE(z80state.pc++);
   }
   goto *CB_INSTRUCTION_TABLE[opcode];
}
DD_PREFIX:
{
   tmrZ80Cpu->CC[0].reg = TStatesTableDDFD[opcode];
   registers = dd_register_table;
   opcode = Z80_FETCH_BYTE(z80state.pc++);
   goto *INSTRUCTION_TABLE[opcode];
}
FD_PREFIX:
{
   tmrZ80Cpu->CC[0].reg = TStatesTableDDFD[opcode];
   registers = fd_register_table;
   opcode = Z80_FETCH_BYTE(z80state.pc++);
   goto *INSTRUCTION_TABLE[opcode];
}
ED_PREFIX:
{
   // tmrZ80Cpu->CC[0].reg = TStatesTableED[opcode];
   registers = register_table;
   opcode = Z80_FETCH_BYTE(z80state.pc++);
   goto *ED_INSTRUCTION_TABLE[opcode];
}
   /* Special/pseudo instruction group. */
ED_UNDEFINED:
{
   exec_done();
}
   // EXEC_DONE:
   //	REG_PORT_OUTCLR1 = PROFILE_PIN;
   return;
}
