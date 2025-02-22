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
#ifndef _BFUNC_H_INCLUDED
#define _BFUNC_H_INCLUDED

#include "rpn.h"

#define OPCODE_MASK   0x80

#define FUNC_TYPE_PROCEDURE   __OPCODE_PRINT
#define FUNC_TYPE_SECONDARY __OPCODE_PEEK
#define FUNC_TYPE_PRIMARY   OPCODE_MASK
#define FUNC_TYPE_NOARG     __OPCODE_INKEY

enum
{
    __OPCODE_REM = FUNC_TYPE_PRIMARY,
    __OPCODE_IF,
    __OPCODE_THEN,
    __OPCODE_GOTO,
    __OPCODE_GOSUB,
    __OPCODE_RETURN,
    __OPCODE_FOR,
    __OPCODE_TO,
    __OPCODE_STEP,
    __OPCODE_NEXT,
    __OPCODE_STOP,
    __OPCODE_RUN,
    __OPCODE_CONT,
    __OPCODE_LIST,
    __OPCODE_NEW,
    __OPCODE_CLEAR,
    __OPCODE_LOAD,
    __OPCODE_SAVE,
    __OPCODE_PRINT,
    __OPCODE_INPUT,
    __OPCODE_PAUSE,
    __OPCODE_SLEEP,
    __OPCODE_CLS,
    __OPCODE_PLOT,
    __OPCODE_DRAW,
    __OPCODE_RECT,
    __OPCODE_CIRCLE,
    __OPCODE_LET,
    __OPCODE_DIM,
    __OPCODE_DEF,
    __OPCODE_SYS,
    __OPCODE_PEEK,
    __OPCODE_POKE,
    __OPCODE_AT,
    __OPCODE_INK,
    __OPCODE_PAPER,
    __OPCODE_VAL$,
    __OPCODE_HEX$,
    __OPCODE_INT,
    __OPCODE_BYTE,
    __OPCODE_WORD,
    __OPCODE_ABS,
    __OPCODE_SIN,
    __OPCODE_COS,
    __OPCODE_TAN,
    __OPCODE_ATN,
    __OPCODE_SQR,
    __OPCODE_RND,
    __OPCODE_LOG,
    __OPCODE_DEG,
    __OPCODE_RAD,
    __OPCODE_MIN,
    __OPCODE_MAX,
    __OPCODE_AND,
    __OPCODE_OR,
    __OPCODE_XOR,
    __OPCODE_SL,
    __OPCODE_SR,
    __OPCODE_INKEY,    
    __OPCODE_ARRAY,
    __OPCODE_DEFFN,
    __OPCODE_LAST
};

typedef enum
{
    BASIC_STAT_OK,
    BASIC_STAT_ERR,
    BASIC_STAT_SKIP,
    BASIC_STAT_JUMP,
    BASIC_STAT_CONT,
    BASIC_STAT_STOP,
} _bas_stat_e;

typedef struct
{
    const char* name;
    _bas_err_e (*func)(_rpn_type_t *);
} _bas_func_t;

extern const _bas_func_t BasicFunction[];

#define bas_func_offset(name) (bas_func_opcode(name) & (~OPCODE_MASK))

uint8_t bas_func_opcode(char *name);
const char *bas_func_name(uint8_t opCode);


#endif // _BFUNC_H_INCLUDED
