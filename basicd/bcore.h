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

#ifndef _BCORE_H_INCLUDED
#define _BCORE_H_INCLUDED

#define BASICD_VESRION      0
#define BASICD_SUBVESRION   50

#define PARSER_MAX_TOKENS   32
#define BASIC_VAR_NAME_LEN  16
#define BASIC_VAR_MAX_COUNT 64
#define BASIC_DEFFN_MAX_ARGS 4

#define BASIC_LINE_LEN 240

#define BASIC_GOSUB_STACK_SIZE 16

#define BASIC_DEFAULT_FILE_NAME "prog.bas"

#include "stdint.h"
#include "rpn.h" // rpn var types
#include "bfunc.h" // status
#include "tstring.h"

#define b_printf tprintf
#define b_sprintf tsnprintf

typedef struct __attribute ((packed))
{
    uint16_t number;
    uint16_t len;
    void *next;
    uint8_t string[0];
} _bas_line_t;

enum _prog_state_e
{
    PROG_STATE_NEW,
    PROG_STATE_RUN,
    PROG_STATE_BREAK,
    PROG_STATE_CONT,
    PROG_STATE_CMDLINE,
    PROG_STATE_DONE
};

typedef struct// __attribute ((packed))
{
    uint16_t number;
    uint16_t nextNum;
    uint8_t statement;
    enum _prog_state_e state:8;
} _bas_ptr_t;

typedef struct// __attribute ((packed))
{
    void *next;
    float limit;
    float step;
    _bas_ptr_t line;
} _bas_loop_t;

typedef struct __attribute ((packed,aligned(4)))
{
    _rpn_type_t value;
    union
    {
        uint8_t argc;       // function arguments count
        uint16_t size[2];   // allocated memory size for complex variables,string - size[0], arrays size[dim 0],size[dim 1]
        _bas_loop_t *loop;  // pointer to loop parameter
    }param;
    void *next;
    char name[0];
} _bas_var_t;

typedef struct
{
    _bas_ptr_t line[BASIC_GOSUB_STACK_SIZE];
    uint8_t ptr;
} _bas_gosub_t;

_bas_var_t *var_get(char *name);
_bas_var_t *var_add(char *name);
uint8_t *basic_line_totext(uint8_t *line);
_bas_line_t *prog_find_line(uint16_t number);
void prog_load(char *progFileName);
void prog_list(void);
void prog_new(void);
void prog_run(uint16_t lineNum);
bool basic_printf(_rpn_type_t *var);
bool prog_add_line(uint16_t number, uint8_t **line);
_bas_err_e __new(_rpn_type_t *param);
_bas_err_e __list(_rpn_type_t *param);
_bas_err_e __load(_rpn_type_t *param);
_bas_err_e __save(_rpn_type_t *param);
_bas_err_e __run(_rpn_type_t *param);
_bas_err_e __cont(_rpn_type_t *param);
_bas_err_e __sys(_rpn_type_t *param);
_bas_err_e __clear(_rpn_type_t *param);

_bas_err_e array_set(char *name,bool init);

extern _bas_stat_e BasicStat;
extern _bas_err_e BasicError;
extern _bas_ptr_t ExecLine;
extern _bas_line_t *BasicProg;
extern _bas_line_t *BasicLineZero;
extern _bas_gosub_t GosubStack;

extern uint8_t tmpBasicLine[BASIC_LINE_LEN];

#endif //_BCORE_H_INCLUDED
