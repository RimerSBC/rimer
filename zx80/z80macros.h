
/* macros.h
 * Helper macros definitions.
 *
 * Copyright (c) 2012-2017 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

/* Shortcuts for flags and registers. */

#define SZC_FLAGS       (Z80_S_FLAG | Z80_Z_FLAG | Z80_C_FLAG)
#define YX_FLAGS        (Z80_Y_FLAG | Z80_X_FLAG)
#define SZ_FLAGS        (Z80_S_FLAG | Z80_Z_FLAG)
#define SZPV_FLAGS      (Z80_S_FLAG | Z80_Z_FLAG | Z80_PV_FLAG)
#define SYX_FLAGS       (Z80_S_FLAG | Z80_Y_FLAG | Z80_X_FLAG)
#define HC_FLAGS        (Z80_H_FLAG | Z80_C_FLAG)

#define A               (z80state.registers.byte[Z80_A])
#define F               (z80state.registers.byte[Z80_F])
#define B               (z80state.registers.byte[Z80_B])
#define C               (z80state.registers.byte[Z80_C])

#define AF              (z80state.registers.word[Z80_AF])
#define BC              (z80state.registers.word[Z80_BC])
#define DE              (z80state.registers.word[Z80_DE])
#define HL              (z80state.registers.word[Z80_HL])
#define SP              (z80state.registers.word[Z80_SP])

#define HL_IX_IY        *((uint16_t *) registers[6])

/* Opcode decoding macros.  Y() is bits 5-3 of the opcode, Z() is bits 2-0,
 * P() bits 5-4, and Q() bits 4-3.
 */

#define Y(opcode)       (((opcode) >> 3) & 0x07)
#define Z(opcode)       ((opcode) & 0x07)
#define P(opcode)       (((opcode) >> 4) & 0x03)
#define Q(opcode)       (((opcode) >> 3) & 0x03)

/* Registers and conditions are decoded using tables in encodings.h.  S() is
 * for the special cases "LD H/L, (IX/Y + d)" and "LD (IX/Y + d), H/L".
 */

#define R(r)            *((uint8_t *) (registers[(r)]))
#define S(s)            *((uint8_t *) register_table[(s)])
#define RR(rr)          *((uint16_t *) registers[(rr) + 8])
#define SS(ss)          *((uint16_t *) registers[(ss) + 12])
#define CC(cc)          ((F ^ XOR_CONDITION_TABLE[(cc)])                \
                         & AND_CONDITION_TABLE[(cc)])
#define DD(dd)          CC(dd)

/* Macros to read constants, displacements, or addresses from code. */

#define READ_N(n)	n = Z80_FETCH_BYTE(z80state.pc++) // elapsed_cycles += 3;

#define READ_NN(nn) {nn = Z80_FETCH_WORD(z80state.pc);\
		z80state.pc += 2;} //elapsed_cycles += 6;

#define READ_D(d)	{d = Z80_FETCH_BYTE(z80state.pc++);} //elapsed_cycles += 3; 

/* Macros to read and write data. */
#define CONTENDED 0
#if CONTENDED
#define READ_BYTE(address)	Z80_READ_BYTE(address) // elapsed_cycles += 3;
#define WRITE_BYTE(address, x)	if (address > 16394 && address < 32768) TSTATES_ADD(10); Z80_WRITE_BYTE((address), (x)) // elapsed_cycles += 3; 
#define READ_WORD(address)  Z80_READ_WORD(address) // elapsed_cycles += 6; 
#define WRITE_WORD(address, x) if (address > 16394 && address < 32768) TSTATES_ADD(10); Z80_WRITE_WORD((address), (x)) //  elapsed_cycles += 6; 
#else
#define READ_BYTE(address)	Z80_READ_BYTE(address) // elapsed_cycles += 3;
#define WRITE_BYTE(address, x)	Z80_WRITE_BYTE((address), (x)) // elapsed_cycles += 3; 
#define READ_WORD(address)  Z80_READ_WORD(address) // elapsed_cycles += 6; 
#define WRITE_WORD(address, x) Z80_WRITE_WORD((address), (x)) //  elapsed_cycles += 6; 
#endif
/* Indirect (HL) and indexed (IX + d) or (IY + d) memory operands read and
 * write macros.
 */
#if 1
#define READ_INDIRECT_HL(x)                                             \
	{                                                                       \
		if (registers == register_table) {			\
			x = READ_BYTE(HL);                                     \
		} else {                                                        \
			int  d;                                              \
			READ_D(d);                                              \
			d += HL_IX_IY;                                          \
			x = READ_BYTE(d);                                      \
		}                                                               \
	}
    
#define WRITE_INDIRECT_HL(x)                                            \
	{                                                                       \
		if (registers == register_table) {			\
			WRITE_BYTE(HL, (x));                                    \
		} else {                                                        \
			int     d;                                              \
			READ_D(d);                                              \
			d += HL_IX_IY;                                          \
			WRITE_BYTE(d, (x));                                     \
		}                                                               \
	}    
#else
#define READ_INDIRECT_HL(x)                                             \
	{                                                                       \
		if (registers == register_table) {			\
			x = READ_BYTE(HL);                                     \
		} else {                                                        \
			int8_t d;                                              \
			READ_D(d);                                              \
			/*d += HL_IX_IY;*/                                          \
			x = READ_BYTE(HL_IX_IY+(signed char)(d));                                      \
		}                                                               \
	}
    
#define WRITE_INDIRECT_HL(x)                                            \
	{                                                                       \
		if (registers == register_table) {			\
			WRITE_BYTE(HL, (x));                                    \
		} else {                                                        \
			signed char d;                                              \
			READ_D(d);                                              \
			/*d += HL_IX_IY;*/                                          \
			WRITE_BYTE((HL_IX_IY+d), (x));                                     \
		}                                                               \
	}    
#endif

/* Stack operation macros. */

#define PUSH(x)                                                         \
	{                                                                       \
		SP -= 2;                                                        \
		WRITE_WORD(SP, (x));                                            \
	}

#define POP(x)                                                          \
	{                                                                       \
		x = READ_WORD(SP);                                             \
		SP += 2;                                                        \
	}

/* Exchange macro. */

#define EXCHANGE(a, b)                                                  \
	{                                                                       \
		int     t;                                                      \
		\
		t = (a);                                                        \
		(a) = (b);                                                      \
		(b) = t;                                                        \
	}

/* 8-bit arithmetic and logic operations. */

#define ADD(x)                                                          \
	{                                                                       \
		int     a, z, c, f;                                             \
		\
		a = A;                                                          \
		z = a + (x);                                                    \
		\
		c = a ^ (x) ^ z;                                                \
		f = c & Z80_H_FLAG;                                             \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		f |= OVERFLOW_TABLE[c >> 7];                                    \
		f |= z >> (8 - Z80_C_FLAG_SHIFT);                               \
		\
		A = z;                                                          \
		F = f;                                                          \
	}

#define ADC(x)                                                          \
	{                                                                       \
		int     a, z, c, f;                                             \
		\
		a = A;                                                          \
		z = a + (x) + (F & Z80_C_FLAG);                                 \
		\
		c = a ^ (x) ^ z;                                                \
		f = c & Z80_H_FLAG;                                             \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		f |= OVERFLOW_TABLE[c >> 7];                                    \
		f |= z >> (8 - Z80_C_FLAG_SHIFT);                               \
		\
		A = z;                                                          \
		F = f;                                                          \
	}

#define SUB(x)                                                          \
	{                                                                       \
		int     a, z, c, f;                                             \
		\
		a = A;                                                          \
		z = a - (x);                                                    \
		\
		c = a ^ (x) ^ z;                                                \
		f = Z80_N_FLAG | (c & Z80_H_FLAG);                              \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		c &= 0x0180;                                                    \
		f |= OVERFLOW_TABLE[c >> 7];                                    \
		f |= c >> (8 - Z80_C_FLAG_SHIFT);                               \
		\
		A = z;                                                          \
		F = f;                                                          \
	}

#define SBC(x)                                                          \
	{                                                                       \
		int     a, z, c, f;                                             \
		\
		a = A;                                                          \
		z = a - (x) - (F & Z80_C_FLAG);                                 \
		\
		c = a ^ (x) ^ z;                                                \
		f = Z80_N_FLAG | (c & Z80_H_FLAG);                              \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		c &= 0x0180;                                                    \
		f |= OVERFLOW_TABLE[c >> 7];                                    \
		f |= c >> (8 - Z80_C_FLAG_SHIFT);                               \
		\
		A = z;                                                          \
		F = f;                                                          \
	}

#define AND(x)                                                          \
	{                                                                       \
		F = SZYXP_FLAGS_TABLE[A &= (x)] | Z80_H_FLAG;                   \
	}

#define OR(x)                                                           \
	{                                                                       \
		F = SZYXP_FLAGS_TABLE[A |= (x)];                                \
	}

#define XOR(x)                                                          \
	{                                                                       \
		F = SZYXP_FLAGS_TABLE[A ^= (x)];                                \
	}

#define CP(x)                                                           \
	{                                                                       \
		int     a, z, c, f;                                             \
		\
		a = A;                                                          \
		z = a - (x);                                                    \
		\
		c = a ^ (x) ^ z;                                                \
		f = Z80_N_FLAG | (c & Z80_H_FLAG);                              \
		f |= SZYX_FLAGS_TABLE[z & 0xff] & SZ_FLAGS;                     \
		f |= (x) & YX_FLAGS;                                            \
		c &= 0x0180;                                                    \
		f |= OVERFLOW_TABLE[c >> 7];                                    \
		f |= c >> (8 - Z80_C_FLAG_SHIFT);                               \
		\
		F = f;                                                          \
	}

#define INC(x)                                                          \
	{                                                                       \
		int     z, c, f;                                                \
		\
		z = (x) + 1;                                                    \
		c = (x) ^ z;                                                    \
		\
		f = F & Z80_C_FLAG;                                             \
		f |= c & Z80_H_FLAG;                                            \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		f |= OVERFLOW_TABLE[(c >> 7) & 0x03];                           \
		\
		(x) = z;                                                        \
		F = f;                                                          \
	}

#define DEC(x)                                                          \
	{                                                                       \
		int     z, c, f;                                                \
		\
		z = (x) - 1;                                                    \
		c = (x) ^ z;                                                    \
		\
		f = Z80_N_FLAG | (F & Z80_C_FLAG);                              \
		f |= c & Z80_H_FLAG;                                            \
		f |= SZYX_FLAGS_TABLE[z & 0xff];                                \
		f |= OVERFLOW_TABLE[(c >> 7) & 0x03];                           \
		\
		(x) = z;                                                        \
		F = f;                                                          \
	}

/* 0xcb prefixed logical operations. */

#define RLC(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) >> 7;                                                   \
		(x) = ((x) << 1) | c;                                           \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define RL(x)                                                           \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) >> 7;                                                   \
		(x) = ((x) << 1) | (F & Z80_C_FLAG);                            \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define RRC(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) & 0x01;                                                 \
		(x) = ((x) >> 1) | (c << 7);                                    \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define RR_INSTRUCTION(x)                                               \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) & 0x01;                                                 \
		(x) = ((x) >> 1) | ((F & Z80_C_FLAG) << 7);                     \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define SLA(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) >> 7;                                                   \
		(x) <<= 1;                                                      \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define SLL(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) >> 7;                                                   \
		(x) = ((x) << 1) | 0x01;                                        \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define SRA(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) & 0x01;                                                 \
		(x) = ((int8_t) (x)) >> 1;  				\
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}

#define SRL(x)                                                          \
	{                                                                       \
		int     c;                                                      \
		\
		c = (x) & 0x01;                                                 \
		(x) >>= 1;                                                      \
		F = SZYXP_FLAGS_TABLE[(x) & 0xff] | c;                          \
	}
