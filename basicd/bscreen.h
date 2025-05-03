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
#ifndef _BSCREEN_H_INCLUDED
#define _BSCREEN_H_INCLUDED

#include "rpn.h"

_bas_err_e __at(_rpn_type_t *p2);
_bas_err_e __ink(_rpn_type_t *p1);
_bas_err_e __paper(_rpn_type_t *p1);
_bas_err_e __over(_rpn_type_t *p1);
_bas_err_e __cls(_rpn_type_t *param);
_bas_err_e __pause(_rpn_type_t *param);
_bas_err_e __sleep(_rpn_type_t *p1);
_bas_err_e __inkey(_rpn_type_t *param);

_bas_err_e __plot(_rpn_type_t *param);
_bas_err_e __draw(_rpn_type_t *param);
_bas_err_e __rect(_rpn_type_t *param);
_bas_err_e __circle(_rpn_type_t *param);
_bas_err_e __bar(_rpn_type_t *param);

#endif //_BSCREEN_H_INCLUDED