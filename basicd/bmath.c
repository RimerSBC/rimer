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
#include <math.h>
#include "freeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "rpn.h"
#include "bcore.h"
#include "bstring.h"
#include "bfunc.h"
#include "bprime.h"
#include "bstring.h"
#include "berror.h"

/// type functions
_bas_err_e __int(_rpn_type_t *param)
{
   int intVar;
   if (param->type < VAR_TYPE_FLOAT)
      if (param->type == VAR_TYPE_STRING)
      {
         char *b = strchr(param->var.str, 'b'); // binary
         if (b) intVar = (int)strtol((char *)(b + 1), NULL, 2);
         else
            intVar = (int)strtol(param->var.str, NULL, 0);
      }
      else return BasicError = BASIC_ERR_TYPE_MISMATCH;
   else
      intVar = (param->type & VAR_TYPE_INT) ? param->var.i : (int)param->var.f;
   rpn_push_queue(RPN_INT(intVar));
   return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __byte(_rpn_type_t *param)
{
   uint8_t intVar;
   if (param->type < VAR_TYPE_FLOAT)
      if (param->type == VAR_TYPE_STRING)
      {
         char *b = strchr(param->var.str, 'b'); // binary
         if (b) intVar = (uint8_t)strtol((char *)(b + 1), NULL, 2);
         else
            intVar = (uint8_t)strtol(param->var.str, NULL, 0);
      }
      else return BasicError = BASIC_ERR_TYPE_MISMATCH;
   else
      intVar = (uint8_t)((param->type & VAR_TYPE_INT) ? param->var.i : param->var.f);
   rpn_push_queue(RPN_BYTE(intVar));
   return BasicError = BASIC_ERR_NONE;
};
/**
 * @brief return a word type of variable(s)
 * @param   single param - simple cast
 *          dual params - creates word from (msb.b,lsb.b)
 *          only byte type is aceptable in dual param implementation
 */
_bas_err_e __word(_rpn_type_t *param)
{
   uint16_t intVar;
   _rpn_type_t *lsb = rpn_pull_queue();
   if (lsb->type) // second parameter
   {
      if ((param->type != VAR_TYPE_BYTE) || (lsb->type != VAR_TYPE_BYTE))
         return BasicError = BASIC_ERR_TYPE_MISMATCH;
      intVar = (uint16_t)((lsb->var.w << 8) + (param->var.w & 0xff));
   }
   else
   {
      if (param->type < VAR_TYPE_FLOAT)
         if (param->type == VAR_TYPE_STRING)
         {
            char *b = strchr(param->var.str, 'b'); // binary
            if (b) intVar = (uint16_t)strtol((char *)(b + 1), NULL, 2);
            else
               intVar = (uint16_t)strtol(param->var.str, NULL, 0);
         }
         else return BasicError = BASIC_ERR_TYPE_MISMATCH;
      else
         intVar = (uint16_t)((param->type & VAR_TYPE_INT) ? param->var.w : param->var.f);
   }
   rpn_push_queue(RPN_WORD(intVar));
   return BasicError = BASIC_ERR_NONE;
};

/// math functions
_bas_err_e __pwr(_rpn_type_t *p2)
{
   _rpn_type_t *p1 = rpn_pull_queue();
   if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
      return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
   else
   {
      float exp = p2->type & VAR_TYPE_FLOAT ? p2->var.f : (float)p2->var.i;
      if (p1->type & VAR_TYPE_FLOAT)
         p1->var.f = pow(p1->var.f, exp);
      else
      {
         p1->var.i = (int)(pow(p1->var.i, exp) + 1e-9);
         switch (p1->type)
         {
         case VAR_TYPE_BYTE:
            p1->var.i = (uint8_t)(p1->var.i);
            break;
         case VAR_TYPE_WORD:
            p1->var.i = (uint16_t)(p1->var.i);
            break;
         default: break;
         }
      }
      rpn_push_queue(*p1);
   }
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __abs(_rpn_type_t *param)
{
   if (param->type < VAR_TYPE_FLOAT)
      return BasicError = BASIC_ERR_TYPE_MISMATCH;
   else if (param->type & VAR_TYPE_FLOAT)
      rpn_push_queue(RPN_FLOAT(param->var.f < 0 ? -param->var.f : param->var.f));
   else
      rpn_push_queue(RPN_INT(param->var.i < 0 ? -param->var.i : param->var.i));
   return BasicError = BASIC_ERR_NONE;
};

_bas_err_e var_float(_rpn_type_t *var)
{
   if (var->type < VAR_TYPE_FLOAT)
      return BasicError = BASIC_ERR_TYPE_MISMATCH;
   if (var->type & VAR_TYPE_INT)
   {
      var->var.f = (float)var->var.i;
      var->type = VAR_TYPE_FLOAT;
   }
   return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __sin(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(sinf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __cos(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(cosf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __tan(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(tanf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __atn(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(atanf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __sqr(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(sqrtf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __rnd(_rpn_type_t *param)
{
   if (param->type < VAR_TYPE_FLOAT) return BasicError = param->type ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
   uint32_t range = (param->type & VAR_TYPE_FLOAT) ? param->var.f : param->var.i;
   // if(seed < 2) seed = 2;
   rpn_push_queue(RPN_FLOAT((float)(rnd(range))));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __log(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(logf(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __deg(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(DEGREES(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __rad(_rpn_type_t *param)
{
   if (var_float(param) != BASIC_ERR_NONE) return BasicError;
   rpn_push_queue(RPN_FLOAT(RADIANS(param->var.f)));
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __min(_rpn_type_t *p1)
{
   _rpn_type_t *p2 = rpn_pull_queue();
   if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
      return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
   else
   {
      float a = p1->type & VAR_TYPE_FLOAT ? p1->var.f : (float)p1->var.i;
      float b = p2->type & VAR_TYPE_FLOAT ? p2->var.f : (float)p2->var.i;
      rpn_push_queue(a < b ? *p1 : *p2);
   }
   return BasicError = BASIC_ERR_NONE;
};
_bas_err_e __max(_rpn_type_t *p1)
{
   _rpn_type_t *p2 = rpn_pull_queue();
   if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
      return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
   else
   {
      float a = p1->type & VAR_TYPE_FLOAT ? p1->var.f : (float)p1->var.i;
      float b = p2->type & VAR_TYPE_FLOAT ? p2->var.f : (float)p2->var.i;
      rpn_push_queue(a > b ? *p1 : *p2);
   }
   return BasicError = BASIC_ERR_NONE;
};
