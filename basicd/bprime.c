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
#include <stdlib.h>
#include "freeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "rpn.h"
#include "banalizer.h"
#include "bcore.h"
#include "bfunc.h"
#include "bstring.h"
#include "bedit.h" // input implementation

_bas_err_e __rem(_rpn_type_t *param)
{
    BasicStat = BASIC_STAT_SKIP;
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __stop(_rpn_type_t *param)
{
    BasicStat = BASIC_STAT_STOP;
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __goto(_rpn_type_t *param)
{
    _rpn_type_t *tmpVar;
    _bas_line_t *bL = BasicProg;
    uint16_t lineNum;
    if (token_eval_expression(param->var.i)) return BasicError;
    tmpVar = rpn_pull_queue();
    if (tmpVar->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_INVALID_LINE;
    lineNum = (tmpVar->type == VAR_TYPE_FLOAT) ? (uint16_t) tmpVar->var.f :  (uint16_t) tmpVar->var.i;
    while (bL && (bL->number != lineNum))  bL = bL->next;
    if (!bL) return BasicError = BASIC_ERR_INVALID_LINE;
    ExecLine.number = lineNum;
    ExecLine.statement = 0;
    BasicStat = BASIC_STAT_JUMP;
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __if(_rpn_type_t *param)
{
    _rpn_type_t *tmpVar;
    if (token_eval_expression(param->var.i)) return BasicError;
    tmpVar = rpn_pull_queue();
    if (tmpVar->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    if ((uint8_t)*bToken->t[bToken->ptr].str != __OPCODE_THEN) return BasicError = BASIC_ERR_NO_THEN;
    BasicStat = tmpVar->var.i ? BASIC_STAT_OK : BASIC_STAT_SKIP;
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __print(_rpn_type_t *param)
{
    _rpn_type_t *var;
    bool head = true;
    if (!param->var.i)
    {
        b_printf("\n");
        return BasicError = BASIC_ERR_NONE;
    }
    while (1)
    {
        if(token_eval_expression(param->var.i)) return BasicError;
        while (1)
        {
            var = rpn_peek_queue(head);
            if (var->type == VAR_TYPE_NONE)// nothing to print
                break;
            if (!head) b_printf("\n"); // print new line when separated by commas
            head = false;
            basic_printf(var);
        }
        if(bToken->t[bToken->ptr].op == '\0' || bToken->t[bToken->ptr].op == ':') break;
        bToken->ptr++;
        param->var.i = '0';
        head = true;
    }
    if (!((bToken->t[bToken->ptr-1].op == ';') && (*bToken->t[bToken->ptr].str == '\0'))) // string termination
        b_printf("\n");
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __input(_rpn_type_t *param)
{
    _bas_var_t *var;
    do
    {
        while (*bToken->t[bToken->ptr].str & OPCODE_MASK)
        {
            if(token_eval_expression(0)) 
                return BasicError;
            bToken->ptr++;
        }
        while (*bToken->t[bToken->ptr].str == '\"')
        {
            bToken->t[bToken->ptr].str++;
            bToken->t[bToken->ptr].str[strlen(bToken->t[bToken->ptr].str)-1] = '\0';
            b_printf("%s",bToken->t[bToken->ptr++].str);
        }
        if (!bToken->t[bToken->ptr].str || *bToken->t[bToken->ptr].str == '\0') return BasicError = BASIC_ERR_MISSING_OPERAND;
        if (*bToken->t[bToken->ptr].str & OPCODE_MASK) return BasicError = BASIC_ERR_RESERVED_NAME;
        if ((var = var_get(bToken->t[bToken->ptr].str)) == NULL)
            if ((var = var_add(bToken->t[bToken->ptr].str)) == NULL) return BasicError; // cannot add a variable
        //strncpy(strTmpBuff,get_basic_line("",false),sizeof(strTmpBuff));
        tstrncpy(strTmpBuff,get_basic_line("",false),sizeof(strTmpBuff));
        switch (var->value.type)
        {
        case VAR_TYPE_FLOAT:
            var->value.var.f = atof(strTmpBuff);
            break;
        case VAR_TYPE_BOOL:
            var->value.var.i = (strchr("0nNfF",*strTmpBuff) || !strlen(strTmpBuff)) ? 0 : 1;
            break;
        case VAR_TYPE_INTEGER:
            var->value.var.i = strtol(strTmpBuff,NULL,0);
            break;
        case VAR_TYPE_STRING:
            if (var_set_string(var,strTmpBuff)) return BasicError;
            break;
        default:
            return BasicError = BASIC_ERR_TYPE_MISMATCH; // you cannot input loop
        }
    }
    while (bToken->t[bToken->ptr++].op == ',');
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __let(_rpn_type_t *param)
{
    _bas_var_t *var;
    _rpn_type_t *tmpVar;
    uint8_t opParam;
    char *varName = bToken->t[bToken->ptr].str;
    BasicError = BASIC_ERR_NONE;
    if (bToken->t[bToken->ptr].op != '=') return BasicError = BASIC_ERR_MISSING_OPERATOR;
    if (bas_func_opcode(varName)) return BasicError = BASIC_ERR_RESERVED_NAME;
    if ((var = var_get(bToken->t[bToken->ptr].str)) == NULL)
        if ((var = var_add(bToken->t[bToken->ptr].str)) == NULL) return BasicError; // cannot add a variable
    opParam = bToken->t[bToken->ptr].op;
    bToken->ptr++;
    if(token_eval_expression(opParam))
        return BasicError;
    else
        tmpVar = rpn_pull_queue();
    switch (var->value.type)
    {
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_LOOP:
        if(tmpVar->type < VAR_TYPE_FLOAT)
            return BasicError = BASIC_ERR_TYPE_MISMATCH;
        else
            var->value.var.f = (tmpVar->type & VAR_TYPE_FLOAT) ? tmpVar->var.f : (float)tmpVar->var.i;
        break;
    case VAR_TYPE_INTEGER:
    case VAR_TYPE_BOOL:
    case VAR_TYPE_BYTE:
        if(tmpVar->type < VAR_TYPE_FLOAT)
            return BasicError = BASIC_ERR_TYPE_MISMATCH;
        else
            var->value.var.i = (tmpVar->type & VAR_TYPE_FLOAT) ? (int32_t)tmpVar->var.f : tmpVar->var.i;
        if (var->value.type == VAR_TYPE_BYTE) var->value.var.i = (uint32_t)var->value.var.i & 0xff;
        break;
    case VAR_TYPE_STRING:
        if(tmpVar->type != VAR_TYPE_STRING)
            return BasicError = BASIC_ERR_TYPE_MISMATCH;
        if (var_set_string(var,tmpVar->var.str)) return BasicError;
        break;
    default:
        break;
    }
    return BasicError = BASIC_ERR_NONE;
};


static _bas_err_e var_get_loop(float *var)
{
    _rpn_type_t *tmpVar;
    uint8_t opParam = bToken->t[bToken->ptr].op;
    bToken->ptr++;
    if(token_eval_expression(opParam))
        return BasicError;
    else
        tmpVar = rpn_pull_queue();
    switch (tmpVar->type)
    {
    case VAR_TYPE_INTEGER:
    case VAR_TYPE_BOOL:
    case VAR_TYPE_BYTE:
        tmpVar->var.f = (float)tmpVar->var.i;
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_LOOP:
        *var = tmpVar->var.f;
        break;
    default:
        return BasicError = BASIC_ERR_TYPE_MISMATCH;
    }
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __for(_rpn_type_t *param)
{
    _bas_var_t *var;
    char *varName = bToken->t[bToken->ptr].str;
    BasicError = BASIC_ERR_NONE;
    if (bToken->t[bToken->ptr].op != '=') return BasicError = BASIC_ERR_MISSING_OPERATOR;
    if (bas_func_opcode(varName)) return BasicError = BASIC_ERR_RESERVED_NAME;
    if ((var = var_get(bToken->t[bToken->ptr].str)) == NULL)
        if ((var = var_add(bToken->t[bToken->ptr].str)) == NULL) return BasicError; // cannot add a variable
    if (var->value.type != VAR_TYPE_LOOP)
    {
        var->value.type = VAR_TYPE_LOOP;
        if ((var->param.loop = pvPortMalloc(sizeof(_bas_loop_t))) == NULL) return BasicError = BASIC_ERR_MEM_OUT;
    }

    if (var_get_loop(&var->value.var.f)) return BasicError;
    var->value.type = VAR_TYPE_LOOP;
    if (!bToken->t[bToken->ptr].op || ((uint8_t)*bToken->t[bToken->ptr].str != __OPCODE_TO))
        return BasicError = BASIC_ERR_INCOMPLETE_FOR;
    if (var_get_loop(&var->param.loop->limit)) return BasicError;
    var->param.loop->step = var->value.var.f < var->param.loop->limit ? 1 : -1;
    if ((uint8_t)*bToken->t[bToken->ptr].str == __OPCODE_STEP)
        if (var_get_loop(&var->param.loop->step)) return BasicError;

    var->param.loop->line.number = bToken->t[bToken->ptr].op == ':' ? ExecLine.number : ExecLine.nextNum;
    var->param.loop->line.statement = bToken->t[bToken->ptr].op == ':' ? ExecLine.statement + 1 : 0;
    var->param.loop->line.state = ExecLine.state;
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __to(_rpn_type_t *param)
{
    return BasicError = BASIC_ERR_INCOMPLETE_FOR;
}

_bas_err_e __step(_rpn_type_t *param)
{
    return BasicError = BASIC_ERR_INCOMPLETE_FOR;
}

_bas_err_e __next(_rpn_type_t *param)
{
    _bas_var_t *var;
    BasicError = BASIC_ERR_NONE;
    if (((var = var_get(bToken->t[bToken->ptr].str)) == NULL) || (var->value.type != VAR_TYPE_LOOP))
        return BasicError = BASIC_ERR_INCOMPLETE_FOR;
    var->value.var.f += var->param.loop->step;
    if (((var->param.loop->step > 0) && (var->value.var.f <= var->param.loop->limit)) || ((var->param.loop->step < 0) && (var->value.var.f >= var->param.loop->limit)))
    {
        ExecLine = var->param.loop->line;
        BasicStat = BASIC_STAT_JUMP;
    }
    else
        BasicStat = BASIC_STAT_OK;
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __gosub(_rpn_type_t *param)
{
    _rpn_type_t *tmpVar;
    _bas_line_t *bL = BasicProg;
    uint16_t lineNum;
    if (GosubStack.ptr >= BASIC_GOSUB_STACK_SIZE) return BasicError = BASIC_ERR_GOSUB_OVERFLOW;
    if (token_eval_expression(param->var.i)) return BasicError;
    tmpVar = rpn_pull_queue();
    if (tmpVar->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_INVALID_LINE;
    GosubStack.line[GosubStack.ptr].number = bToken->t[bToken->ptr].op == ':' ? ExecLine.number : ExecLine.nextNum;
    GosubStack.line[GosubStack.ptr++].statement = bToken->t[bToken->ptr].op == ':' ? ExecLine.statement + 1 : 0;
    lineNum = (tmpVar->type == VAR_TYPE_FLOAT) ? (uint16_t) tmpVar->var.f :  (uint16_t) tmpVar->var.i;
    while (bL && (bL->number != lineNum))  bL = bL->next;
    if (!bL) return BasicError = BASIC_ERR_INVALID_LINE;
    ExecLine.number = lineNum;
    ExecLine.statement = 0;
    BasicStat = BASIC_STAT_JUMP;
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __return(_rpn_type_t *param)
{
    if (!GosubStack.ptr) return BasicError = BASIC_ERR_RETURN_NO_GOSUB;
    ExecLine = GosubStack.line[--GosubStack.ptr];
    BasicStat = BASIC_STAT_JUMP;
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __dim(_rpn_type_t *param)
{
    _bas_var_t *var;
    char *varName = bToken->t[bToken->ptr].str;
    if (bToken->t[bToken->ptr].op != '[') return BasicError = BASIC_ERR_PAR_MISMATCH;
    if (bas_func_opcode(varName)) return BasicError = BASIC_ERR_RESERVED_NAME;
    if ((var = var_get(bToken->t[bToken->ptr].str)) != NULL) return BasicError = BASIC_ERR_ARRAY_REDEFINE;
    if ((var = var_add(bToken->t[bToken->ptr++].str)) == NULL) return BasicError; // cannot add a variable
    var->param.size[0] = (uint16_t) strtol(bToken->t[bToken->ptr].str,NULL,0);
    var->param.size[1] = (bToken->t[bToken->ptr].op == ',') ? (uint16_t) strtol(bToken->t[++bToken->ptr].str,NULL,0) : 0;
    if ((var->param.size[0] == 0 && var->param.size[1] == 0) || (bToken->t[bToken->ptr].op != ']'))
        return BasicError = BASIC_ERR_ARRAY_DIMENTION;
    else
    {
        size_t arraySize;
        if (var->value.type == VAR_TYPE_STRING)
        {
            if (!var->param.size[1]) return BasicError = BASIC_ERR_ARRAY_DIMENTION;
            var->param.size[1]++; // add a termination byte
            arraySize = var->param.size[0] * var->param.size[1];
        }
        else
            arraySize = var->param.size[0] * (var->param.size[1] ? var->param.size[1] : 1) * ((var->value.type == VAR_TYPE_BYTE) ? 1 : sizeof(var->value));
        if ((var->value.var.array = (char *)pvPortMalloc(arraySize)) == NULL) return BasicError = BASIC_ERR_MEM_OUT;
    }
    switch (var->value.type)
    {
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_LOOP:
        var->value.type = VAR_TYPE_ARRAY_FLOAT;
        break;
    case VAR_TYPE_INTEGER:
    case VAR_TYPE_BOOL:
        var->value.type = VAR_TYPE_ARRAY_INTEGER;
        break;
    case VAR_TYPE_BYTE:
        var->value.type = VAR_TYPE_ARRAY_BYTE;
        break;
    case VAR_TYPE_STRING:
        var->value.type = VAR_TYPE_ARRAY_STRING;
        break;
    default:
        break;
    }
//    b_printf("Array allocated: dim0 = %d, dim1 = %d, size = %lu\n",var->param.size[0],var->param.size[1],(var->param.size[0] * (var->param.size[1] ? var->param.size[1] : 1) * ((var->value.type == VAR_TYPE_ARRAY_BYTE) ? 1 : sizeof(var->value))));
    bToken->ptr++;
    if (bToken->t[bToken->ptr].op == '=')
    {
        if (array_set(varName,true)) return BasicError;
    }
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __def(_rpn_type_t *param)
{
    _bas_var_t *var;
    uint8_t i,strPtr=0;
    char argVarName[BASIC_VAR_NAME_LEN];
    char argIndName[2] = "0";
    char *argument[BASIC_DEFFN_MAX_ARGS];
    char *varName = bToken->t[bToken->ptr].str;
    _bas_tok_list_t *tmpTok;
    if (bToken->t[bToken->ptr].op != '(') return BasicError = BASIC_ERR_PAR_MISMATCH;
    if (bas_func_opcode(varName)) return BasicError = BASIC_ERR_RESERVED_NAME;
    if ((var = var_get(varName)) != NULL) return BasicError = BASIC_ERR_DEFFN_REDEFINE;
    if ((var = var_add(varName)) == NULL) return BasicError; // cannot add a variable
    var->param.argc = 0; // number of arguments
    do // get function arguments
    {
        bToken->ptr++;
        if(*bToken->t[bToken->ptr].str)
        {
            strncpy(argVarName,varName,BASIC_VAR_NAME_LEN-1);
            argIndName[0] = '0'+var->param.argc;
            strncat(argVarName,argIndName,BASIC_VAR_NAME_LEN-1);
            argument[var->param.argc] = bToken->t[bToken->ptr].str;
            var->param.argc++;
            if (var->param.argc >= BASIC_DEFFN_MAX_ARGS) return BASIC_ERR_DEFFN_ARGUMENTS;
            if (var_get(argVarName) != NULL) return BasicError = BASIC_ERR_VAR_REDEFINE;
            if (var_add(argVarName) == NULL) return BasicError; // cannot add a variable
        }
        if (bToken->t[bToken->ptr].op == '\0') return BasicError = BASIC_ERR_PAR_MISMATCH;
    }
    while (bToken->t[bToken->ptr].op != ')');
    if (bToken->t[++bToken->ptr].op != '=')
        return BasicError = BASIC_ERR_MISSING_EQUAL;
    bToken->ptr++;
    for (i=bToken->ptr; i<PARSER_MAX_TOKENS; i++)
    {
        varName = bToken->t[i].str; // store the token in varName variable
        if (var->param.argc)
            for (uint8_t n=0; n<var->param.argc; n++)
                if (!strcmp(varName,argument[n]))
                {
                    argVarName[strlen(argVarName)-1] = '0'+n;
                    varName = argVarName; // replace the arg var name with the "local" one
                    break;
                }
        bToken->t[i].str = (char *)0+strPtr; // store the token pointer offsets to the temp buffer
        if (strPtr >= BASIC_STRING_LEN-3) return BASIC_ERR_STRING_LENGTH;
        while(*varName)
            strTmpBuff[strPtr++] = *(varName++);
        strTmpBuff[strPtr++] = '\0';
        if ((!bToken->t[i].op) || (bToken->t[i].op == ':') || (bToken->t[i].op == ';')) break;
    }
    i -= bToken->ptr - 1; // i contains the number of tokens
    if ((tmpTok = pvPortMalloc(sizeof(_bas_tok_list_t))) == NULL) return BasicError = BASIC_ERR_MEM_OUT; // allocate memory for the tokens list
    if ((tmpTok->t = (_bas_token_t *)pvPortMalloc(sizeof(_bas_token_t)*i)) == NULL) return BasicError = BASIC_ERR_MEM_OUT; // allocate memory for the tokens
    if ((tmpTok->t[0].str = pvPortMalloc(strPtr)) == NULL) return BasicError = BASIC_ERR_MEM_OUT; // allocate memory for the tokens' string
    memcpy(tmpTok->t[0].str,strTmpBuff,strPtr); // copy tokens
    tmpTok->ptr=0;
    for (uint8_t n=0; n<i; n++)
    {
        tmpTok->t[n].str = (char *)((long)bToken->t[bToken->ptr].str + (long)tmpTok->t[0].str); // adding offset only
        tmpTok->t[n].op = bToken->t[bToken->ptr++].op;
    }
#if 0 // Print tokenized strings
    b_printf("Allocated: tokList(%lu), tokens(%lu), string(%d). Total bytes: %lu\n",sizeof(_bas_tok_list_t),sizeof(_bas_token_t)*i,strPtr,sizeof(_bas_tok_list_t)+sizeof(_bas_token_t)*i+strPtr);
    b_printf("tmpTok list:\n");
    for (uint8_t i=0; i<PARSER_MAX_TOKENS; i++)
    {
        b_printf("\"%s\", \'%c\'\n",tmpTok->t[i].str,tmpTok->t[i].op);
        if (!tmpTok->t[i].op) break;
    }
#endif
    var->value.var.deffn = tmpTok;
    switch (var->value.type)
    {
    case VAR_TYPE_FLOAT:
        var->value.type = VAR_TYPE_DEFFN_FLOAT;
        break;
    case VAR_TYPE_INTEGER:
    case VAR_TYPE_BOOL:
        var->value.type = VAR_TYPE_DEFFN_INTEGER;
        break;
    case VAR_TYPE_BYTE:
        var->value.type = VAR_TYPE_DEFFN_BYTE;
        break;
    case VAR_TYPE_STRING:
        var->value.type = VAR_TYPE_DEFFN_STRING;
        break;
    default:
        break;
    }
    return BasicError = BASIC_ERR_NONE;
}
