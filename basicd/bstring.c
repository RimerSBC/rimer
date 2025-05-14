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
#include "bcore.h"
#include "banalizer.h"
#include "bstring.h"

char strTmpBuff[BASIC_STRING_LEN];
static char strVarBuff[BASIC_STRING_LEN];
static char strNumBuff[36];
_bas_err_e __val$(_rpn_type_t *param)
{
    switch (param->type)
    {
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_LOOP:
      b_sprintf(strNumBuff,sizeof(strNumBuff),"%s",tftoa(param->var.f,0));
      break;
    case VAR_TYPE_INT:
    case VAR_TYPE_BYTE:
    case VAR_TYPE_WORD:
      b_sprintf(strNumBuff,sizeof(strNumBuff),"%d", param->var.i);
      break;
    case VAR_TYPE_BOOL:
      b_sprintf(strNumBuff,sizeof(strNumBuff),"%s", param->var.i ? "true" : "false");
      break;
    case VAR_TYPE_STRING:
      b_sprintf(strNumBuff,sizeof(strNumBuff),"%s", param->var.str);
      break;
    default:
      return BasicError = BASIC_ERR_TYPE_MISMATCH;
    }
    rpn_push_queue(RPN_STR(strNumBuff));
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __hex$(_rpn_type_t *param)
{
    b_sprintf(strNumBuff,sizeof(strNumBuff),(param->type == VAR_TYPE_BYTE) ? "%2X" : (param->type == VAR_TYPE_WORD) ? "%4X" : "%8X",param->var.i);
    rpn_push_queue(RPN_STR(strNumBuff));
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __bin$(_rpn_type_t *param)
{
    uint8_t i,len = (param->type == VAR_TYPE_BYTE) ? 8 : (param->type == VAR_TYPE_WORD) ? 16 : 32;
    uint32_t mask=(0x01<<(len-1));
    for (i=0;i<len;i++,mask>>=1)
        strNumBuff[i]=param->var.w&mask?'1':'0';
    strNumBuff[i]='\0';
    rpn_push_queue(RPN_STR(strNumBuff));
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e string_add(char *str1, char *str2)
{
    if((strlen(str1) + strlen(str2)) > BASIC_STRING_LEN-2) return BasicError = BASIC_ERR_STRING_LENGTH;
    *strTmpBuff = '\0';
    strcpy(strTmpBuff,str1);
    strcat(strTmpBuff,str2);
    strcpy(strVarBuff,strTmpBuff);
    rpn_push_queue(RPN_STR(strVarBuff));
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e var_set_string(_bas_var_t *var, char *str)
{
    if(var->param.size[0] < (strlen(str)+1))
    {
        if (var->param.size[0]) vPortFree(var->value.var.str);
        var->param.size[0] = strlen(str)+1;
        if ((var->value.var.str = (char *)pvPortMalloc(var->param.size[0])) == NULL)
            return BasicError = BASIC_ERR_MEM_OUT;
        //b_printf("\n *** Reallocate %d bytes for %s\n", var->size, var->value.var.str);
    }
    tstrncpy(var->value.var.str,str,var->param.size[0]);
    return BasicError = BASIC_ERR_NONE;
}
	