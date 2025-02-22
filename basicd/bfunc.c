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
#include <stdint.h>
#include "freeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "rpn.h"
#include "bcore.h"
#include "banalizer.h"
#include "bfunc.h"
#include "bprime.h"
#include "bmath.h"
#include "bscreen.h"
#include "bstring.h"
#include "berror.h"

static _bas_err_e __basic_dummy(_rpn_type_t *param);
static _bas_err_e __array(_rpn_type_t *param);
static _bas_err_e __deffn(_rpn_type_t *param);
static _bas_err_e __peek(_rpn_type_t *param);
static _bas_err_e __poke(_rpn_type_t *param);

const _bas_func_t BasicFunction[] =
{
    /// --- basic primary operators (procedures)
    {"rem",__rem},
    /// --- program flow ctrol
    {"if",__if},
    {"then",__basic_dummy},
    {"goto",__goto},
    {"gosub",__gosub},
    {"return",__return},
    {"for",__for},
    {"to",__to},
    {"step",__step},
    {"next",__next},
    /// --- interpreter control
    {"stop",__stop},
    {"run",__run},
    {"cont",__cont},
    {"list",__list},
    {"new",__new},
    {"clear",__clear},
    {"load",__load},
    {"save",__save},
    /// --- input/output
    {"print",__print},
    {"input",__input},
    {"pause",__pause},
    {"sleep",__sleep},
    {"cls",__cls},
    /// --- graphics
    {"plot",__plot},
    {"draw",__draw},
    {"rect",__rect},
    {"circle",__circle},       
    /// --- data manipulation
    {"let",__let},
    {"dim",__dim},
    {"def",__def},
    {"sys",__sys},
    /// all operators after this point should be executed in context of other operators
    /// --- basic secondary operators (functions)
    /// --- input/output/format
    {"peek",__peek},
    {"poke",__poke},
    {"at",__at},
    {"ink",__ink},
    {"paper",__paper},
    /// --- string/type
    {"val$",__val$},
    {"hex$",__hex$},
    /// --- data type
    {"int",__int},
    {"byte",__byte},
    {"word",__word},
    /// --- math
    {"abs",__abs},
    {"sin",__sin},
    {"cos",__cos},
    {"tan",__tan},
    {"atn",__atn},
    {"sqr",__sqr},
    {"rnd",__rnd},
    {"log",__log},
    {"deg",__deg},
    {"rad",__rad},
    {"min",__min},
    {"max",__max},
    /// --- logic
    {"and#",__and},
    {"or#",__or},
    {"xor#",__xor},
    {"sl#",__sl},
    {"sr#",__sr},
    /// --- no argument functions
    {"inkey",__inkey},
    /// data type 
    {" ",__array}, /// access to arrays element
    {" ",__deffn}, /// implement define function
    //  {NULL,NULL}
};

uint8_t bas_func_opcode(char *name)
{
    for (uint8_t fCnt = 0; fCnt < (__OPCODE_LAST - OPCODE_MASK); fCnt++)
        if (!strcmp(name,BasicFunction[fCnt].name)) return OPCODE_MASK + fCnt;
    return 0;
}

const char *bas_func_name(uint8_t opCode)
{
    if (opCode >= __OPCODE_LAST) return "noOp";
    return BasicFunction[opCode - OPCODE_MASK].name;
}

static _bas_err_e __basic_dummy(_rpn_type_t *param)
{
    BasicError = BASIC_ERR_UNKNOWN_FUNC;
    return BasicError = BASIC_ERR_NONE;
};

static _bas_err_e __array(_rpn_type_t *param)
{
    _bas_var_t *array;
    if (!rpn_find_queue(VAR_TYPE_ARRAY)) return BASIC_ERR_QUEUE_EMPTY; // should not happened
    array = (rpn_peek_queue(false))->var.array; // get array
    bool stringArray = array->value.type == VAR_TYPE_ARRAY_STRING ? true : false;
    _rpn_type_t *var;
    uint16_t tmpArrayPtr, arrayPtr = 0;
    for (uint8_t i=0; i < ((array->param.size[1] && !stringArray) ? 2 : 1); i++)
    {
        if ((var = rpn_peek_queue(false))->type  == VAR_TYPE_NONE) return BasicError = BASIC_ERR_ARRAY_DIMENTION;
        if (!var->type || (var->type < VAR_TYPE_FLOAT)) 
            return BasicError = BASIC_ERR_TYPE_MISMATCH;
        tmpArrayPtr = (var->type < VAR_TYPE_INTEGER) ? (uint16_t)var->var.f : (uint16_t)var->var.i;
        if (tmpArrayPtr >= array->param.size[i]) // check range
            return BasicError = BASIC_ERR_ARRAY_OUTOFRANGE;
        arrayPtr = arrayPtr*array->param.size[1] + tmpArrayPtr;
    }
    if (rpn_peek_queue(false)->type != VAR_TYPE_NONE) return BasicError = BASIC_ERR_ARRAY_DIMENTION; // there is something in stack, too much dimentions
    void *data;
    if (stringArray)
        data = array->value.var.array + arrayPtr*array->param.size[1];
    else
        data = array->value.var.array + arrayPtr*(array->value.type == VAR_TYPE_ARRAY_BYTE ? 1 : 4);
    switch (array->value.type)
    {
    case VAR_TYPE_ARRAY_BYTE:
        rpn_push_queue(RPN_INT(*(uint8_t *)data));
        break;
    case VAR_TYPE_ARRAY_INTEGER:
        rpn_push_queue(RPN_INT(*(int32_t *)data));
        break;
    case VAR_TYPE_ARRAY_FLOAT:
        rpn_push_queue(RPN_FLOAT(*(float *)data));
        break;
    case VAR_TYPE_ARRAY_STRING:
        rpn_push_queue(RPN_STR(data));
        break;
    default:
        return BasicError = BASIC_ERR_TYPE_MISMATCH;
    }
    return BasicError = BASIC_ERR_NONE;
};

static _bas_err_e __deffn(_rpn_type_t *param)
{
    char argVarName[BASIC_VAR_NAME_LEN];
    _bas_var_t *func;
    if (!rpn_find_queue(VAR_TYPE_DEFFN)) return BASIC_ERR_QUEUE_EMPTY; // should not happened
    func = (rpn_peek_queue(false))->var.deffn; // get function tokens
    if (func->param.argc) // there are arguments
    {
        uint8_t indexChr = strlen(func->name);
        _bas_var_t *var;
        strcpy(argVarName,func->name);
        argVarName[indexChr+1]=0;
        for (uint8_t i=0;i<func->param.argc;i++)
        {
            argVarName[indexChr] = '0'+i;
            if ((var = var_get(argVarName)) == NULL) return BasicError = BASIC_ERR_UNKNOWN_VAR;
            var->value = *rpn_peek_queue(false);
            if (var->value.type == VAR_TYPE_NONE) return BasicError = BASIC_ERR_FEW_ARGUMENTS;
        }
        if (rpn_peek_queue(false)->type != VAR_TYPE_NONE) return BasicError = BASIC_ERR_MANY_ARGUMENTS;
    }
    ((_bas_tok_list_t *)func->value.var.deffn)->ptr = -1;
    ((_bas_tok_list_t *)func->value.var.deffn)->parCnt = 0;
    tok_list_push(func->value.var.deffn);
    return BasicError = BASIC_ERR_NONE;
};

static _bas_err_e __peek(_rpn_type_t *p1)
{
    uint32_t data;
    if (p1->type != VAR_TYPE_INTEGER) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    memcpy(&data,(uint8_t *)(p1->var.w),sizeof(data));
    rpn_push_queue(RPN_INT(data));
    return BasicError = BASIC_ERR_NONE;
}
static _bas_err_e __poke(_rpn_type_t *p2)
{
    _rpn_type_t *p1 = rpn_pull_queue();
    if ((p1->type != VAR_TYPE_INTEGER) || (p2->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    else
    {
        switch(p2->type)
        {
            case VAR_TYPE_LOOP:
            case VAR_TYPE_FLOAT:
                    *(uint32_t *)(p1->var.w) = (uint32_t)p2->var.f;
                break;
            case VAR_TYPE_WORD:
                    *(uint16_t *)(p1->var.w) = (uint16_t)p2->var.w;
                break;            
            case VAR_TYPE_BYTE:
                    *(uint8_t *)(p1->var.w) = (uint8_t)p2->var.w;
                break;
            default:
                *(uint32_t *)(p1->var.w) = p2->var.w;
        }
            
    }
    return BasicError = BASIC_ERR_NONE;
}
