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
#ifndef _BPRIME_H_INCLUDED
#define _BPRIME_H_INCLUDED

#include "rpn.h"

_bas_err_e __rem(_rpn_type_t *param);
_bas_err_e __stop(_rpn_type_t *param);
_bas_err_e __print(_rpn_type_t *param);
_bas_err_e __input(_rpn_type_t *param);
_bas_err_e __let(_rpn_type_t *param);
_bas_err_e __for(_rpn_type_t *param);
_bas_err_e __to(_rpn_type_t *param);
_bas_err_e __step(_rpn_type_t *param);
_bas_err_e __next(_rpn_type_t *param);
_bas_err_e __goto(_rpn_type_t *param);
_bas_err_e __if(_rpn_type_t *param);
_bas_err_e __gosub(_rpn_type_t *param);
_bas_err_e __return(_rpn_type_t *param);
_bas_err_e __dim(_rpn_type_t *param);
_bas_err_e __def(_rpn_type_t *param);

#endif // _BPRIME_H_INCLUDED