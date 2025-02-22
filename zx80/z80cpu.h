/* z80emu.h
 * z80user.h to customize the emulator to your need.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#ifndef __Z80EMU_INCLUDED__
#define __Z80EMU_INCLUDED__

#include "stdint.h"

#define OPCODE_LD_A_I 0x57
#define OPCODE_LD_I_A 0x47

#define OPCODE_LDI 0xa0
#define OPCODE_LDIR 0xb0
#define OPCODE_CPI 0xa1
#define OPCODE_CPIR 0xb1

#define OPCODE_RLD 0x6f

#if defined(Z80_CATCH_RETI) && defined(Z80_CATCH_RETN)
#define OPCODE_RETI 0x4d
#endif

#define OPCODE_INI 0xa2
#define OPCODE_INIR 0xb2
#define OPCODE_OUTI 0xa3
#define OPCODE_OTIR 0xb3

/* If Z80_STATE's status is non-zero, the emulation has been stopped for some
 * reason other than emulating the requested number of cycles. See z80config.h.
 */

enum
{
   Z80_STATUS_HALT = 1,
   Z80_STATUS_DI,
   Z80_STATUS_EI,
   Z80_STATUS_RETI,
   Z80_STATUS_RETN,
   Z80_STATUS_ED_UNDEFINED,
   Z80_STATUS_PREFIX
};

/* The main registers are stored inside Z80_STATE as an union of arrays named
 * registers. They are referenced using indexes. Words are stored in the
 * endianness of the host processor. The alternate set of word registers AF',
 * BC', DE', and HL' is stored in the alternates member of Z80_STATE, as an
 * array using the same ordering.
 */

enum
{
   Z80_F,
   Z80_A,
   Z80_C,
   Z80_B,
   Z80_E,
   Z80_D,
   Z80_L,
   Z80_H,
   Z80_IXL,
   Z80_IXH,
   Z80_IYL,
   Z80_IYH,
};
enum
{
   Z80_AF,
   Z80_BC,
   Z80_DE,
   Z80_HL,
   Z80_IX,
   Z80_IY,
   Z80_SP,
   Z80_AF_,
   Z80_BC_,
   Z80_DE_,
   Z80_HL_,
   Z80_PC,
};

/* Z80's flags. */

#define Z80_S_FLAG_SHIFT 7
#define Z80_Z_FLAG_SHIFT 6
#define Z80_Y_FLAG_SHIFT 5
#define Z80_H_FLAG_SHIFT 4
#define Z80_X_FLAG_SHIFT 3
#define Z80_PV_FLAG_SHIFT 2
#define Z80_N_FLAG_SHIFT 1
#define Z80_C_FLAG_SHIFT 0

#define Z80_S_FLAG (1 << Z80_S_FLAG_SHIFT)
#define Z80_Z_FLAG (1 << Z80_Z_FLAG_SHIFT)
#define Z80_Y_FLAG (1 << Z80_Y_FLAG_SHIFT)
#define Z80_H_FLAG (1 << Z80_H_FLAG_SHIFT)
#define Z80_X_FLAG (1 << Z80_X_FLAG_SHIFT)
#define Z80_PV_FLAG (1 << Z80_PV_FLAG_SHIFT)
#define Z80_N_FLAG (1 << Z80_N_FLAG_SHIFT)
#define Z80_C_FLAG (1 << Z80_C_FLAG_SHIFT)

#define Z80_P_FLAG_SHIFT Z80_PV_FLAG_SHIFT
#define Z80_V_FLAG_SHIFT Z80_PV_FLAG_SHIFT
#define Z80_P_FLAG Z80_PV_FLAG
#define Z80_V_FLAG Z80_PV_FLAG

/* Z80's three interrupt modes. */

enum
{
   Z80_INTERRUPT_MODE_0,
   Z80_INTERRUPT_MODE_1,
   Z80_INTERRUPT_MODE_2
};

/* Z80 processor's state. You may add your own members if needed. However, it
 * is rather suggested to use the context pointer passed to the emulation
 * functions for that purpose. See z80user.h.
 */

typedef struct __attribute__((__packed__)) Z80_STATE
{
   union
   {
      uint8_t byte[14];
      uint16_t word[7];
   } registers;
   uint16_t alternates[4];
   uint16_t pc;
   uint8_t i, r, im;
   uint8_t iff1, iff2;
   uint8_t status;
} Z80_STATE;

/* Initialize processor's state to power-on default. */

extern void Z80Reset(Z80_STATE *state);

/* Trigger an interrupt according to the current interrupt mode and return the
 * number of cycles elapsed to accept it. If maskable interrupts are disabled,
 * this will return zero. In interrupt mode 0, data_on_bus must be a single
 * byte opcode.
 */

extern void Z80Interrupt(void);
/* Trigger a non maskable interrupt, then return the number of cycles elapsed
 * to accept it.
 */

extern void Z80NonMaskableInterrupt(void);

/* Execute single instruction. The user macros
 * (see z80user.h) control the emulation.
 */
#define z80_cycle() TC0_Handler()
extern volatile uint8_t addWaitStates;
extern uint16_t clkZ80div;
extern Z80_STATE z80state;
extern uint8_t dataOnBus;

void z80_set_clock(uint32_t fClkHz);
uint32_t z80_get_clock(void);
void z80_init(void);
void z80cpu_run(void);
void z80cpu_stop(void);
#endif
