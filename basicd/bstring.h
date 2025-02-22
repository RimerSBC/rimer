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
#ifndef _BSTRING_H_INCLUDED
#define _BSTRING_H_INCLUDED

#include "bcore.h"

#define BASIC_STRING_LEN 128
_bas_err_e __val$(_rpn_type_t *param);
_bas_err_e __hex$(_rpn_type_t *param);
_bas_err_e string_add(char *str1, char *str2);
_bas_err_e var_set_string(_bas_var_t *var, char *str);

extern char strTmpBuff[BASIC_STRING_LEN];

#endif //_BSTRING_H_INCLUDED