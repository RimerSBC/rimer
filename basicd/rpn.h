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
#ifndef _RPN_H_INCLUDED
#define _RPN_H_INCLUDED

#include "stdint.h"
#include "stdbool.h"
#include "berror.h"

#define RPN_QUEUE_LEN   32
#define RPN_STACK_LEN   32

#define DEGREES(rad)    ((rad) * 180.0 / M_PI)
#define RADIANS(deg)    ((deg) * M_PI / 180.0)
/// Math operators
#define OPERATOR_PWR        '^'
#define OPERATOR_MUL        '*'
#define OPERATOR_DIV        '/'
#define OPERATOR_MOD        '%'
#define OPERATOR_PLUS       '+'
#define OPERATOR_MINUS      '-'
/// Logic operators
#define OPERATOR_AND        '&'
#define OPERATOR_OR         '|'
#define OPERATOR_NOT        '!'
/// Condition operators
#define OPERATOR_MORE       '>'
#define OPERATOR_LESS       '<'
#define OPERATOR_EQUAL      '='
#define OPERATOR_MORE_EQ    '~'
#define OPERATOR_NOT_EQ     '@'
#define OPERATOR_LESS_EQ    '`'

typedef enum
{
    VAR_TYPE_NONE,
    ///      Defined function
    VAR_TYPE_DEFFN = 0x08,      // Pointer to a defined function
    VAR_TYPE_DEFFN_STRING,      // multi-byte variables
    VAR_TYPE_DEFFN_FLOAT,
    VAR_TYPE_DEFFN_INTEGER,
    VAR_TYPE_DEFFN_BYTE,   
    VAR_TYPE_ARRAY = 0x10,      // Pointer to an array
    VAR_TYPE_ARRAY_STRING,      // multi-byte variables
    VAR_TYPE_ARRAY_FLOAT,
    VAR_TYPE_ARRAY_INTEGER,
    VAR_TYPE_ARRAY_BYTE,
    VAR_TYPE_STRING = 0x20,     // string,  variable name ended with '$'
    VAR_TYPE_FLOAT = 0x40,      // numeric variables
    VAR_TYPE_LOOP,              // for loop variable, .size points to the loop descriptor  
    VAR_TYPE_INTEGER = 0x80,    // integer, variable name ended with '#'
    VAR_TYPE_BYTE,              // byte,  variable name ended with '_'
    VAR_TYPE_WORD,              // 16 bit unsigned  
    VAR_TYPE_BOOL,              // boolean
} _var_type_e;

typedef struct// __attribute ((packed,aligned(4)))
{
    _var_type_e type;
    union
    {
        float       f;
        int32_t     i;
        uint32_t    w;
        char*       str;
        void*       array;
        void*       deffn;
    } var;
} _rpn_type_t;

#define RPN_FLOAT(x)    ((_rpn_type_t){.type = VAR_TYPE_FLOAT,.var.f = x})
#define RPN_INT(x)      ((_rpn_type_t){.type = VAR_TYPE_INTEGER,.var.i = x})
#define RPN_BYTE(x)     ((_rpn_type_t){.type = VAR_TYPE_BYTE,.var.i = x})
#define RPN_WORD(x)     ((_rpn_type_t){.type = VAR_TYPE_WORD,.var.i = x})
#define RPN_STR(x)      ((_rpn_type_t){.type = VAR_TYPE_STRING,.var.str = x})

_bas_err_e rpn_push_queue(_rpn_type_t var);
_rpn_type_t *rpn_pull_queue(void);
_rpn_type_t *rpn_peek_queue(bool head);
bool rpn_find_queue(_var_type_e varType);

_bas_err_e rpn_push_stack(uint8_t op);
uint8_t rpn_pull_stack(void);
uint8_t rpn_peek_stack_last(void);
_bas_err_e rpn_eval(uint8_t op);

void rpn_print_queue(bool newline);
void rpn_print_stack(bool newline);

void rpn_purge_queue(void);
void rpn_purge_stack(void);

#endif //_RPN_H_INCLUDED
