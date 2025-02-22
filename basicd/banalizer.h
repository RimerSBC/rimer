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
#ifndef _BANALIZER_H_INCLUDED
#define _BANALIZER_H_INCLUDED

#include "bcore.h"

#define is_digit(cc) (((cc)>='0' && (cc)<='9') ? true : false)

typedef struct __attribute ((packed))
{
    char *str;
    char op;
} _bas_token_t;

typedef struct __attribute ((packed))
{
    _bas_token_t *t;
    uint8_t ptr;
    uint8_t parCnt;
} _bas_tok_list_t;

bool tok_list_push(_bas_tok_list_t *tokensList);
bool tok_list_pull(void);

extern _bas_tok_list_t *bToken;
_bas_var_t *var_get(char *name);
bool tokenizer(char *str);
_bas_err_e token_eval_expression(uint8_t opParam);

#endif //_BANALIZER_H_INCLUDED
