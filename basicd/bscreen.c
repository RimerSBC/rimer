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
#include "freeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "graph.h"
#include "rpn.h"
#include "bcore.h"
#include "bstring.h"
#include "bfunc.h"
#include "bprime.h"
#include "bstring.h"
#include "berror.h"
#include "banalizer.h"
#include "keyboard.h"
#include "uterm.h"

uint16_t lastX = 0, lastY = 0;

_bas_err_e __at(_rpn_type_t *p2)
{
    _rpn_type_t *p1 = rpn_pull_queue();
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    uint8_t x = (uint8_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    uint8_t y = (uint8_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    if (!(x && y)) return BasicError = BASIC_ERR_VAR_OUTOFRANGE;
    x -= 1;
    y -= 1;
    if (!(x < uTerm.cols && y < uTerm.lines)) return BasicError = BASIC_ERR_VAR_OUTOFRANGE;
    uTerm.cursorCol = x;
    uTerm.cursorLine = y;
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __ink(_rpn_type_t *p1)
{
    if (p1->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    uint8_t c = ((uint8_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i));// & 0x07;
    if (p1->type == VAR_TYPE_BYTE) 
        uTerm.fgColour = c; 
    else
        text_fg_colour(c);
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __paper(_rpn_type_t *p1)
{
    if (p1->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    uint8_t c = ((uint8_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i));// & 0x07;
    if (p1->type == VAR_TYPE_BYTE) 
        uTerm.bgColour = c; 
    else
        text_bg_colour(c);
    return BasicError = BASIC_ERR_NONE;
};
/*
_bas_err_e __over(_rpn_type_t *p1)
{
    if (p1->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    set_pixel_mode(p1->var.i ? true : false);
    return BasicError = BASIC_ERR_NONE;
};*/

_bas_err_e __cls(_rpn_type_t *param)
{
    text_cls();
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __pause(_rpn_type_t *param)
{
    char cc;
    keyboard_wait(&cc);
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __sleep(_rpn_type_t *param)
{
    _rpn_type_t *var;
    if (!param->var.i) return BasicError = BASIC_ERR_FEW_ARGUMENTS;
    if(token_eval_expression(param->var.i)) return BasicError;
    var = rpn_pull_queue();
    if (var->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
    uint32_t t = (int32_t)(var->type & VAR_TYPE_FLOAT ? var->var.f : var->var.i);
    vTaskDelay(t);
    return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __inkey(_rpn_type_t *param)
{
    char cc = 0,ct;
    if (keyboard_getch(&cc))
    {
        if (cc == 27) // process cursor keys
        {
            if (keyboard_getch(&ct)) cc = ct;
        }
    }
    return rpn_push_queue(RPN_BYTE(cc));
}

/// primary screen function
//int16_t funcNumParam[6];
struct
{
   int16_t var[6];
   uint8_t count;
} funcParam;

_bas_err_e get_num_params(uint8_t opParam)
{
     _rpn_type_t *tmpVar;
    uint8_t i;
    if (token_eval_expression(opParam)) return BasicError;
    for (i = 0;i < 6;i++)
    {
        tmpVar = rpn_peek_queue(!i);
        if (tmpVar->type == VAR_TYPE_NONE) break;
        if (tmpVar->type < VAR_TYPE_FLOAT) return BasicError = BASIC_ERR_TYPE_MISMATCH;
        funcParam.var[i] = (int16_t)(tmpVar->type & VAR_TYPE_FLOAT ? tmpVar->var.f : tmpVar->var.i);
        if (funcParam.var[i] < 0) return BasicError = BASIC_ERR_VAR_OUTOFRANGE;
    }
    funcParam.count = i;
    return BasicError = i ? BASIC_ERR_NONE : BASIC_ERR_FEW_ARGUMENTS;
}

_bas_err_e __plot(_rpn_type_t *param)
{
  if (get_num_params(param->var.i)) return BasicError;
  switch(funcParam.count)
  {
      case 2: x_pixel = put_pixel;
            break;
      case 3: x_pixel = funcParam.var[2] ? put_pixel : put_xpixel;
            break;             
      default:  return BasicError = BASIC_ERR_FEW_ARGUMENTS;
  }
  lastX = funcParam.var[0];
  lastY = funcParam.var[1];
  x_pixel(funcParam.var[0],funcParam.var[1],uTerm.fgColour);
  return BasicError = BASIC_ERR_NONE;
}


_bas_err_e __draw(_rpn_type_t *param)
{
  bool xorMode = false;
  if (get_num_params(param->var.i)) return BasicError;
  switch(funcParam.count)
  {
      case 3: xorMode = funcParam.var[2] ? true : false;
      case 2: x_pixel = xorMode ? put_xpixel : put_pixel;
              line(lastX,lastY,funcParam.var[0],funcParam.var[1],uTerm.fgColour);
              lastX = funcParam.var[0];
              lastY = funcParam.var[1];
              break;
      case 5: xorMode = funcParam.var[4] ? true : false;
      case 4: x_pixel = xorMode ? put_xpixel : put_pixel;
              line(funcParam.var[0],funcParam.var[1],funcParam.var[2],funcParam.var[3],uTerm.fgColour);
              lastX = funcParam.var[3];
              lastY = funcParam.var[4];
              break;
      default:  return BasicError = BASIC_ERR_FEW_ARGUMENTS;
  }
  return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __rect(_rpn_type_t *param)
{
  bool xorMode = false;
  bool fill = false;
  if (get_num_params(param->var.i)) return BasicError;
  switch(funcParam.count)
  {
      case 6: xorMode = funcParam.var[5] ? true : false;
      case 5: fill = funcParam.var[4] ? true : false;
      case 4: x_pixel = xorMode ? put_xpixel : put_pixel;
            if (fill)
               rect_fill(funcParam.var[0],funcParam.var[1],funcParam.var[2],funcParam.var[3],uTerm.fgColour);
            else
               rect(funcParam.var[0],funcParam.var[1],funcParam.var[2],funcParam.var[3],uTerm.fgColour);
            break;
     default:  return BasicError = BASIC_ERR_FEW_ARGUMENTS;
  }
  return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __circle(_rpn_type_t *param)
{
  bool xorMode = false;
  bool fill = false;
  if (get_num_params(param->var.i)) return BasicError;
  switch(funcParam.count)
  {
      case 5: xorMode = funcParam.var[4] ? true : false;
      case 4: fill = funcParam.var[3] ? true : false;
      case 3: x_pixel = xorMode ? put_xpixel : put_pixel;
            if (fill)
                circle_fill(funcParam.var[0],funcParam.var[1],funcParam.var[2],uTerm.fgColour);
            else
                circle(funcParam.var[0],funcParam.var[1],funcParam.var[2],uTerm.fgColour);
            break;
     default:  return BasicError = BASIC_ERR_FEW_ARGUMENTS;
  }
  return BasicError = BASIC_ERR_NONE;
}

/*
_bas_err_e __plot(_rpn_type_t *p2)
{
    _rpn_type_t *p1 = rpn_pull_queue();
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    lastX = (uint16_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    lastY = (uint16_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    //if (!(x && y)) return BasicError = BASIC_ERR_VAR_OUTOFRANGE;
    put_pixel(lastX,lastY,uTerm.fgColour);
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __line(_rpn_type_t *p4)
{
    _rpn_type_t *p3 = rpn_pull_queue();
    _rpn_type_t *p2 = rpn_pull_queue();
    _rpn_type_t *p1 = rpn_pull_queue();
    uint16_t x;
    uint16_t y;
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT) || (p3->type < VAR_TYPE_FLOAT) || (p4->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type && p3->type && p4->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    x = (uint16_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    y = (uint16_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    lastX = (uint16_t)(p3->type & VAR_TYPE_FLOAT ? p3->var.f : p3->var.i);
    lastY = (uint16_t)(p4->type & VAR_TYPE_FLOAT ? p4->var.f : p4->var.i);
    line(x,y,lastX,lastY,uTerm.fgColour);
    return BasicError = BASIC_ERR_NONE;
};


_bas_err_e __lineto(_rpn_type_t *p2)
{
    _rpn_type_t *p1 = rpn_pull_queue();
    uint16_t x = lastX,y = lastY;
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    lastX = (uint16_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    lastY = (uint16_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    line(x,y,lastX,lastY,uTerm.fgColour);
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __rect(_rpn_type_t *p4)
{
    _rpn_type_t *p3 = rpn_pull_queue();
    _rpn_type_t *p2 = rpn_pull_queue();
    _rpn_type_t *p1 = rpn_pull_queue();
    uint16_t sizeX;
    int16_t sizeY;
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT) || (p3->type < VAR_TYPE_FLOAT) || (p4->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type && p3->type && p4->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    lastX = (uint16_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    lastY = (uint16_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    sizeX = (uint16_t)(p3->type & VAR_TYPE_FLOAT ? p3->var.f : p3->var.i);
    sizeY = (int16_t)(p4->type & VAR_TYPE_FLOAT ? p4->var.f : p4->var.i);
    if (sizeY < 0)
    {
        rect_fill(lastX,lastY,sizeX,-sizeY,uTerm.fgColour);
    }
    else
        rect(lastX,lastY,sizeX,sizeY,uTerm.fgColour);
    return BasicError = BASIC_ERR_NONE;
};

_bas_err_e __circle(_rpn_type_t *r)
{
    _rpn_type_t *p2 = rpn_pull_queue();
    _rpn_type_t *p1 = rpn_pull_queue();
    int16_t radius;
    if ((p1->type < VAR_TYPE_FLOAT) || (p2->type < VAR_TYPE_FLOAT) || (r->type < VAR_TYPE_FLOAT))
        return BasicError = (p1->type && p2->type && r->type) ? BASIC_ERR_TYPE_MISMATCH : BASIC_ERR_FEW_ARGUMENTS;
    lastX = (uint16_t)(p1->type & VAR_TYPE_FLOAT ? p1->var.f : p1->var.i);
    lastY = (uint16_t)(p2->type & VAR_TYPE_FLOAT ? p2->var.f : p2->var.i);
    radius = (int16_t)(r->type & VAR_TYPE_FLOAT ? r->var.f : r->var.i);
    if (radius < 0)
    {
        circle_fill(lastX,lastY,-radius,uTerm.fgColour);
    }
    else
        circle(lastX,lastY,radius,uTerm.fgColour);
    return BasicError = BASIC_ERR_NONE;
};
*/