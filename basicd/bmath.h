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
#ifndef _BMATH_H_INCLUDED
#define _BMATH_H_INCLUDED

#include "rpn.h"
/// type
_bas_err_e __int(_rpn_type_t *param);
_bas_err_e __byte(_rpn_type_t *param);
_bas_err_e __word(_rpn_type_t *param);
/// math
_bas_err_e __abs(_rpn_type_t *param);
_bas_err_e __sin(_rpn_type_t *param);
_bas_err_e __cos(_rpn_type_t *param);
_bas_err_e __tan(_rpn_type_t *param);
_bas_err_e __atn(_rpn_type_t *param);
_bas_err_e __sqr(_rpn_type_t *param);
_bas_err_e __rnd(_rpn_type_t *param);
_bas_err_e __log(_rpn_type_t *param);
_bas_err_e __deg(_rpn_type_t *param);
_bas_err_e __rad(_rpn_type_t *param);
_bas_err_e __min(_rpn_type_t *param);
_bas_err_e __max(_rpn_type_t *param);

_bas_err_e __and(_rpn_type_t *p1);
_bas_err_e __or(_rpn_type_t *p1);
_bas_err_e __xor(_rpn_type_t *p1);
_bas_err_e __sl(_rpn_type_t *p1);
_bas_err_e __sr(_rpn_type_t *p1);

#endif //_BMATH_H_INCLUDED