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

#include "bcore.h"
#include "banalizer.h"
#include "bedit.h"
#include "bfunc.h"
#include "bhighlight.h"
#include "bprog_rom.h"
#include "bsp.h"
#include "bstring.h"
#include "ctype.h"
#include "ff.h"
#include "freeRTOS.h"
#include "keyboard.h"
#include "rpn.h"
#include "rshell.h"
#include "task.h"
#include "uterm.h"
#include <stdlib.h>
#include <string.h>

const struct
{
   const char *name;
   char op;
} BasicOperators[8] = {
    {"and", OPERATOR_AND},
    {"or", OPERATOR_OR},
    {"not", OPERATOR_NOT},
    {">=", OPERATOR_MORE_EQ},
    {"<>", OPERATOR_NOT_EQ},
    {"<=", OPERATOR_LESS_EQ},
    {"<<", OPERATOR_BWSL},
    {">>", OPERATOR_BWSR},
};
const char *OP_STR = " `=<>~!@%^&*()-+|\\,./;:\r\n";

_bas_stat_e BasicStat;
_bas_line_t *BasicProg = NULL;
_bas_ptr_t ExecLine = {0, 0, 0, PROG_STATE_NEW};
_bas_ptr_t ContLine;
_bas_var_t *BasicVars = NULL;
_bas_line_t *BasicLineZero = NULL;
static _bas_line_t *bL;
static _bas_line_t *contbL = NULL;

uint8_t tmpBasicLine[BASIC_LINE_LEN];
_bas_gosub_t GosubStack = {.ptr = 0};
/*
_bas_var_t BasicConstants[] =
{
    {.name = "pi",.next = &BasicConstants[1],.value.var.f = 3.14159,.value.type = VAR_TYPE_FLOAT},
    {.name = "e",.next = NULL,.value.var.f = 2.71828,.value.type = VAR_TYPE_FLOAT},
};*/

void basic_putch(char cc)
{
   if (cc == '\n')
      ut_new_line(1);
   else
   {
      glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(cc));
      if (uTerm.cursorCol < uTerm.cols - 1)
         uTerm.cursorCol++;
      else
         ut_new_line(1);
   }
}

_bas_var_t *var_get(char *name)
{
   _bas_var_t *varPtr = BasicVars; // BasicConstants;
   if (!varPtr)
      return NULL;
   while (1)
   {
      if (!strcmp(name, varPtr->name))
         return varPtr;
      if ((varPtr = varPtr->next) == NULL)
         return NULL;
   }
}

_bas_var_t *var_add(char *name)
{
   _bas_var_t *varPtr = BasicVars; // BasicConstants;
   uint8_t nameLen = strlen(name);
   char *typeQ = (char *)(name + nameLen - 1);          // get var type qualifier: xxx - number(float),xxx$ - string, xxx.i/w/b integer/word/byte
   uint16_t varSize = sizeof(_bas_var_t) + nameLen + 1; // include string terminator
   if (varSize & 0x3)
      varSize = (varSize & ~(0x03)) + 4; // allign to 4
   if (!BasicVars)                       // new var
   {
      BasicVars = pvPortMalloc(varSize);
      varPtr = BasicVars;
   }
   else
   {
      while (varPtr->next)
         varPtr = varPtr->next;
      varPtr->next = pvPortMalloc(varSize);
      if (varPtr->next == NULL)
      {
         BasicError = BASIC_ERR_MEM_OUT;
         return NULL;
      }
      varPtr = varPtr->next;
   }
   varPtr->next = NULL; ///--- already null;
   strcpy(varPtr->name, name);
   varPtr->value.type = VAR_TYPE_FLOAT; 
   if (nameLen > 1) // single character variable is always float
      switch (*typeQ--)
      {
      case '$':
         varPtr->value.type = VAR_TYPE_STRING;
         break;
      case 'b':
         if (*typeQ == '.') varPtr->value.type = VAR_TYPE_BYTE;
         break;
      case 'w':
         if (*typeQ == '.') varPtr->value.type = VAR_TYPE_WORD;
         break;
      case 'i':
         if (*typeQ == '.') varPtr->value.type = VAR_TYPE_INT;
         break;
      default:
         varPtr->value.type = VAR_TYPE_FLOAT;
      }
   varPtr->param.size[0] = 0;
   return varPtr;
}

_bas_line_t *prog_find_line(uint16_t number)
{
   _bas_line_t *line = BasicProg;
   while (line)
      if (line->number == number)
         return line;
      else
         line = line->next;
   return NULL;
}

uint8_t *basic_line_totext(uint8_t *line) /// look for an opcode name and replace it with function name
{
   bool quoted = false;
   bool remarked = false;
   const char *opName = NULL;
   uint8_t destPtr = 0;
   while (*line)
   {
      if (destPtr > (BASIC_LINE_LEN - 2))
         break;

      if (*line == '\'' || remarked) // REM found
      {
         while (*line && (destPtr < (BASIC_LINE_LEN - 1)))
            tmpBasicLine[destPtr++] = *line++;
         break;
      }

      if (*line == '\"')
      {
         if (quoted && !(*(char *)(line - 1) == '\\'))
            quoted = false;
         else
            quoted = true;
      }

      if (quoted)
      {
         tmpBasicLine[destPtr++] = *line++;
         continue;
      }

      if (*line >= OPCODE_MASK)
         opName = bas_func_name(*line);
      for (uint8_t i = 0; i < 8 && !opName; i++)
         if (*line == BasicOperators[i].op)
            opName = BasicOperators[i].name;
      if (opName)
      {
         if (*line == __OPCODE_REM)
            remarked = true; // REM found
         while (*opName)
            tmpBasicLine[destPtr++] = *(opName++);
         if (*line == OPERATOR_NOT)
            tmpBasicLine[destPtr++] = ' ';
         opName = NULL;
      }
      else
         tmpBasicLine[destPtr++] = *line;
      line++;
   }
   tmpBasicLine[destPtr] = '\0';
   return tmpBasicLine;
}

uint8_t *basic_line_preprocess(
    uint8_t *line) /// look for a function name and replace it with op code
{
   bool quoted = false;
   bool remarked = false;
   uint8_t opCode = 0;
   uint8_t head = 0,
           tail = 0; // head points to the last character of the temp string and
                     // tail points to beginning of the last word
   while (*line == ' ')
      line++; // skip leading spaces
   while (*line)
   {
      /** process quoted text*/
      if (*line == '\"')
      {
         if (quoted && !(*(char *)(line - 1) == '\\'))
         {
            quoted = false;
            tmpBasicLine[head++] = *(line++);
            tail = head;
            continue;
         }
         else
            quoted = true;
      }
      if (quoted)
      {
         tmpBasicLine[head++] = *(line++);
         continue;
      }
      /** skip remarked */
      if ((*line == '\'') || remarked)
      {
         while (*line && !(*line == '\r' || *line == '\n'))
            tmpBasicLine[head++] = *(line++);
         break;
      }
      /** check for delimeter */
      if ((!*line) || (strchr(OP_STR, *line))) // delimeter found
      {
         if (((*line == '=') || (*line == '>') || (*line == '<')) &&
             ((*(line - 1) == '<') || (*(line - 1) == '>'))) // process <>, >=, <=, <<, and >>
         {
            tail--;
            tmpBasicLine[head++] = *(line++);
         }
         tmpBasicLine[head] = '\0'; // terminate the temporary string
         for (uint8_t i = 0; i < 8 && !opCode; i++)
            if (!strcmp((char *)&tmpBasicLine[tail], BasicOperators[i].name))
               opCode = BasicOperators[i].op;
         if (!opCode)
         {
            if ((tmpBasicLine[tail] == '?') && (tmpBasicLine[tail + 1] == '\0'))
               opCode = bas_func_opcode("print");
            else
               opCode = bas_func_opcode((char *)&tmpBasicLine[tail]);
         }
         if (opCode) // reserved name found
         {
            tmpBasicLine[tail++] = opCode;
            head = tail;
            if (opCode == __OPCODE_REM)
               remarked = true;
            opCode = 0;
         }
         tail = head + 1;
         /** check for eol */
         if (*line == '\r' || *line == '\n') //
            break;
      }
      if (*line == ' ') // remove multiple spaces
         while (*(line + 1) == ' ')
            line++;
      if (*line == '\"')
         quoted = true;
      tmpBasicLine[head++] = *(line++);
   }
   tmpBasicLine[head] = '\0';
   return tmpBasicLine;
}

bool prog_add_line(uint16_t number, uint8_t **line)
{
   uint16_t lineLen = 0;
   if (BasicLineZero == NULL)
   {
      BasicLineZero = pvPortMalloc(sizeof(_bas_line_t));
      memset(BasicLineZero, 0x00, sizeof(_bas_line_t));
   }
   _bas_line_t *bLine = number ? prog_find_line(number) : BasicLineZero; // start new or update existing
   if (number && **line == '\n')                                         // delete line
   {
      if (bLine == NULL)
         return true;
      if (BasicProg->number == number)
         BasicProg = BasicProg->next;
      else
      {
         for (_bas_line_t *blSeek = BasicProg; blSeek != NULL; blSeek = blSeek->next)
            if (((_bas_line_t *)(blSeek->next))->number == number)
            {
               blSeek->next = ((_bas_line_t *)(blSeek->next))->next;
               break;
            }
      }
      vPortFree(bLine);
      return true;
   }
   /// add/update a basic line
   uint8_t *blString = basic_line_preprocess(*line);
   uint16_t blStrLen = strlen((char *)blString) + 1; // include string terminator
                                                     //  if (blStrLen & 0x3)
                                                     //     blStrLen = (blStrLen & ~(0x03)) + 4; // allign to 4
                                                     /// b_printf("add line %d,%d\n", sizeof(_bas_line_t), blStrLen);
   if (bLine == NULL)                                // create new
   {
      if ((bLine = pvPortMalloc(sizeof(_bas_line_t) + blStrLen)) == NULL) // add new line + string length
         return false;
      bLine->number = number;
      bLine->next = NULL;
      bLine->len = blStrLen;
      if (BasicProg == NULL)
      {
         BasicProg = bLine; // Start a new program
      }
      else
      {
         if (bLine->number < BasicProg->number) // swap if got smaller line number than the first one
         {
            bLine->next = BasicProg;
            BasicProg = bLine;
         }
         else
         {
            _bas_line_t *blSeek = BasicProg;
            while (blSeek->next != NULL)
            {
               if (bLine->number > ((_bas_line_t *)(blSeek->next))->number)
                  blSeek = blSeek->next;
               else // the new line has lower number
               {
                  bLine->next = blSeek->next; // Insert the new line before higher line
                  blSeek->next = bLine;
                  break;
               }
            }
            if (blSeek->next == NULL) // add to the end
               blSeek->next = bLine;
         }
      }
   }
   bLine->number = number;
   while ((*line)[lineLen] > '\r')
      lineLen++;
   if (lineLen >= BASIC_LINE_LEN - 1)
      lineLen = BASIC_LINE_LEN - 2;
   if ((bLine->len == 0) || (bLine->len < blStrLen)) // reallocate
   {
      _bas_line_t tmpBline = *bLine;
      _bas_line_t *prevLine = BasicProg;
      while (prevLine) // find previous line pointing to bLine
      {
         if (prevLine->next == bLine)
            break;
         prevLine = prevLine->next;
      }
      vPortFree(bLine);
      if ((bLine = pvPortMalloc(sizeof(_bas_line_t) + blStrLen)) == NULL) // reallocate line + string length
         return false;
      *bLine = tmpBline;
      bLine->len = blStrLen;
      if (!number)
         BasicLineZero = bLine;
      if (prevLine)
         prevLine->next = bLine;
   }
   strcpy((char *)bLine->string, (char *)blString);
   bLine->len = lineLen;
   *line += lineLen;
   return true;
}

_bas_err_e __new(_rpn_type_t *param)
{
   _bas_line_t *blSeek = BasicProg;
   __clear(NULL);
   while (blSeek)
   {
      vPortFree(blSeek);
      blSeek->len = 0;
      blSeek = blSeek->next;
   }
   BasicProg = NULL;
   if (BasicLineZero)
   {
      vPortFree(BasicLineZero);
      BasicLineZero = NULL;
   }
   b_printf("Free mem: %d bytes\n", xPortGetFreeHeapSize());
   ExecLine = (_bas_ptr_t){0, 0, PROG_STATE_NEW};
   return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __clear(_rpn_type_t *param)
{
   _bas_var_t *nextVar, *delVar = BasicVars;
   while (delVar)
   {
      nextVar = delVar->next;
      if (delVar->value.type == VAR_TYPE_LOOP) // release loop structure
         vPortFree(delVar->param.loop);
      else if (delVar->value.type & (VAR_TYPE_ARRAY | VAR_TYPE_STRING)) // release loop structure
         vPortFree(delVar->value.var.array);
      else if (delVar->value.type & VAR_TYPE_DEFFN) // release deffn structure
      {
         vPortFree(((_bas_tok_list_t *)(delVar->value.var.deffn))->t[0].str);
         vPortFree(((_bas_tok_list_t *)(delVar->value.var.deffn))->t);
         vPortFree(delVar->value.var.deffn);
      }
      vPortFree(delVar);
      delVar = nextVar;
   }
   BasicVars = NULL;
   memset(&GosubStack, 0x00, sizeof(GosubStack));
   return BasicError = BASIC_ERR_NONE;
}

void bas_list(uint16_t begin, uint16_t count)
{
   char str[8];
   // uint8_t lineCnt=0;
   _bas_line_t *bL = BasicProg;
   if (bL == NULL) return;
   text_cls();
   while (bL->next) // find the line
   {
      if (bL->number >= begin) break;
      bL = bL->next;
   }
   while (bL != NULL && count)
   {
      if (uTerm.cursorLine > uTerm.lines - 3)
      {
         b_printf("more(y/n)?");
         if (keyboard_wait("yY"))
            text_cls();
         else
         {
            uTerm.cursorCol = 0;
            b_printf("          ");
            uTerm.cursorCol = 0;
            return;
         }
      }
      if (bL->number)
         b_sprintf(str, sizeof(str), "%d ", bL->number);
      highlight(str); // print line number
      highlight((char *)basic_line_totext(bL->string));
      if (uTerm.cursorCol)
      {
         b_printf("\n");
         //  lineCnt++;
      }
      bL = bL->next;
      count--;
   }
}

_bas_err_e __list(_rpn_type_t *param)
{
   uint16_t progLine = 0;
   uint16_t lineCount = 32768;
   if (param->var.i && (token_eval_expression(0) == BASIC_ERR_NONE))
   {
      _rpn_type_t *var = rpn_peek_queue(true);
      if (var->type >= VAR_TYPE_FLOAT)
         progLine = var->type < VAR_TYPE_INT ? (uint16_t)var->var.f : (uint16_t)var->var.i;
      var = rpn_peek_queue(false);
      if (var->type >= VAR_TYPE_FLOAT)
         lineCount = var->type < VAR_TYPE_INT ? (uint16_t)var->var.f : (uint16_t)var->var.i;
   }
   bas_list(progLine, lineCount);
   return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __save(_rpn_type_t *filename)
{
   FIL progFile;
   char str[8];
   _bas_line_t *bL = BasicProg;
   char *progStr;

   if (filename->type != VAR_TYPE_STRING)
   {
      if (token_eval_expression(0))
         return BasicError;
      filename = rpn_peek_queue(true);
   }
   if (filename->type != VAR_TYPE_STRING)
      return BasicError = (filename->type == VAR_TYPE_NONE) ? BASIC_ERR_FEW_ARGUMENTS : BASIC_ERR_TYPE_MISMATCH;

   if (!*filename->var.str)
   {
      filename->var.str = BASIC_DEFAULT_FILE_NAME;
   }
   if (f_open(&progFile, filename->var.str, FA_WRITE) == FR_OK)
   {
      basic_message(BASIC_MSG_WARNING, "Overwrite \"%s\" (y/n)?", filename->var.str);
      if (!keyboard_wait("yY"))
      {
         f_close(&progFile);
         b_printf("skipped...\n");
         return BasicError = BASIC_ERR_NONE;
      }
   }
   else if (f_open(&progFile, filename->var.str, FA_WRITE | FA_CREATE_NEW) != FR_OK)
      return BasicError = BASIC_ERR_FILE_CANT_OPEN;
   f_truncate(&progFile);
   while (bL != NULL)
   {
      if (bL->number)
         b_sprintf(str, sizeof(str), "%d ", bL->number);
      progStr = (char *)basic_line_totext(bL->string);
      strncat(progStr, "\n", BASIC_LINE_LEN - 2);
      f_puts(str, &progFile);
      f_puts(progStr, &progFile);
      b_printf("%s%s", str, progStr);
      bL = bL->next;
   }
   f_close(&progFile);
   b_printf("\"%s\" saved\n", filename->var.str);
   return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __load(_rpn_type_t *filename)
{
   FIL prog;
   char *lineBuffer;
   if (filename->type != VAR_TYPE_STRING)
   {
      if (token_eval_expression(0))
         return BasicError;
      filename = rpn_peek_queue(true);
   }
   if (filename->type != VAR_TYPE_STRING)
      return BasicError = (filename->type == VAR_TYPE_NONE) ? BASIC_ERR_FEW_ARGUMENTS : BASIC_ERR_TYPE_MISMATCH;

   if (!*filename->var.str)
   {
      filename->var.str = BASIC_DEFAULT_FILE_NAME;
   }
   if ((f_open(&prog, filename->var.str, FA_READ) != FR_OK))
      return BasicError = BASIC_ERR_FILE_NOT_FOUND;
   lineBuffer = pvPortMalloc(1024);
   if (lineBuffer == NULL)
   {
      f_close(&prog);
      return BasicError = BASIC_ERR_MEM_OUT;
   }
   BasicError = BASIC_ERR_NONE;
   while (f_gets(lineBuffer, 1023, &prog))
   {
      uint8_t *str = (uint8_t *)lineBuffer;
      if (str[strlen((char *)str) - 1] > '\r') // line is not terminated with \r or \n, end of file
         strcat((char *)str, "\r");
      while ((*str == ' ') || (*str == '\t'))
         str++;
      if (*str < '0')
         continue; // skip lines strarting with # or control char
      if ((!BasicError) && (ExecLine.number = (uint16_t)strtol(
                                (char *)str, (char **)&str, 10)) == 0)
         BasicError = BASIC_ERR_LOAD_NONUMBER;
      if ((!BasicError) && prog_find_line(ExecLine.number))
         BasicError = BASIC_ERR_LOAD_DUPLICATE;
      if ((!BasicError) && !prog_add_line(ExecLine.number, &str))
         BasicError = BASIC_ERR_MEM_OUT;
      if (BasicError)
         break;
   }
   vPortFree(lineBuffer);
   f_close(&prog);

   return BasicError;
}

bool basic_printf(_rpn_type_t *var)
{
   switch (var->type)
   {
   case VAR_TYPE_FLOAT:
   case VAR_TYPE_LOOP:
      b_printf("%s", tftoa(var->var.f, 3));
      break;
   case VAR_TYPE_INT:
   case VAR_TYPE_BYTE:
   case VAR_TYPE_WORD:
      b_printf("%d", var->var.i);
      break;
   case VAR_TYPE_BOOL:
      b_printf("%s", var->var.i ? "true" : "false");
      break;
   case VAR_TYPE_STRING:
      b_printf("%s", var->var.str);
      break;
   default:
      return false;
   }
   return true;
}

_bas_stat_e basic_line_eval(void)
{
   uint8_t firstOp;
   char *str;
   while (1)
   {
      if (!*bToken->t[bToken->ptr].str && !bToken->t[bToken->ptr].op)
         return BasicStat = BASIC_STAT_SKIP; // empty line (rem found)
      firstOp = bToken->t[bToken->ptr].op;
      switch (firstOp)
      {
      case '=': // variable assignement
         BasicStat = BASIC_STAT_OK;
         if (BasicFunction[__OPCODE_LET & ~OPCODE_MASK].func(&RPN_INT(0)))
            return BasicStat = BASIC_STAT_ERR;
         if (BasicStat)
            return BasicStat; // other than error
         break;
      case '[': // array assignement
         if (array_set(bToken->t[bToken->ptr].str, false))
            return BasicStat = BASIC_STAT_ERR;
         break;
      default:
         str = bToken->t[bToken->ptr].str;
         if (*str == '\'')
            *str = (char)__OPCODE_REM;
         else if ((uint8_t)*str < OPCODE_MASK)
         {
            _bas_var_t *var;
            if ((var = var_get(str)) && (var->value.type &= VAR_TYPE_DEFFN))
            {
               if (token_eval_expression(0))
                  return BasicStat = BASIC_STAT_ERR;
               break;
            }
            BasicError = BASIC_ERR_UNKNOWN_FUNC;
            return BasicStat = BASIC_STAT_ERR;
         }
         uint8_t opCode = *bToken->t[bToken->ptr].str;
         if (firstOp != ':')
            bToken->ptr++;
         if (opCode < __OPCODE_LAST)
         {
            BasicStat = BASIC_STAT_OK;
            if (opCode < FUNC_TYPE_SECONDARY)
            {
               if (BasicFunction[opCode & (~OPCODE_MASK)].func(
                       &RPN_INT(firstOp)))
                  return BasicStat = BASIC_STAT_ERR;
            }
            else
            {
               if (bToken->ptr)
                  bToken->ptr--; // evaluate secondary function
               if (token_eval_expression(0))
                  return BasicStat = BASIC_STAT_ERR;
               while (basic_printf(rpn_pull_queue()))
                  b_printf("\n"); // print RPN queue
            }
            if (BasicStat)
               return BasicStat; // other than error
         }
      }
      if (bToken->t[bToken->ptr].op == ';')
      {
         BasicError = BASIC_ERR_INVALID_DELIMITER;
         return BasicStat = BASIC_STAT_ERR;
      }
      if (bToken->t[bToken->ptr].op == ':')
         ExecLine.statement++;
      else if (!bToken->t[bToken->ptr].op || ((uint8_t)*bToken->t[bToken->ptr].str != __OPCODE_THEN))
         return BasicStat = BASIC_STAT_OK;
      bToken->ptr++;
   }
   return BasicStat = BASIC_STAT_OK;
}

_bas_err_e __run(_rpn_type_t *param)
{
   if (ExecLine.state == PROG_STATE_RUN)
      return BASIC_ERR_RUN_ERROR;

   if (param->type == VAR_TYPE_BYTE)
   {
      if (BasicLineZero == NULL)
      {
         BasicLineZero = pvPortMalloc(sizeof(_bas_line_t));
         memset(BasicLineZero, 0x00, sizeof(_bas_line_t));
      }
      bL = BasicLineZero;
   }
   else
   {
      uint16_t progLine = 0;
      if (param->var.i && (token_eval_expression(0) == BASIC_ERR_NONE))
      {
         _rpn_type_t *var = rpn_peek_queue(true);
         if (var->type >= VAR_TYPE_FLOAT)
            progLine = var->type < VAR_TYPE_INT ? (uint16_t)var->var.f : (uint16_t)var->var.i;
      }
      bL = BasicProg;
      if (progLine)
      {
         while (bL && (bL->number < progLine))
            bL = bL->next; // find the executuion line (or the next one)
         if (bL == NULL)
         {
            BasicStat = BASIC_STAT_ERR;
            return BasicError = BASIC_ERR_INVALID_LINE;
         }
      }
      //__clear(NULL);
      ExecLine.state = PROG_STATE_RUN;
      BasicStat = BASIC_STAT_CONT;
      if (param->type != VAR_TYPE_WORD) // call from system
        return BasicError = BASIC_ERR_NONE; // will continue from the "command line" instance
   }
   ExecLine.statement = 0;
   keyboard_break(); // skip previous breaks

   while (bL)
   {
      tokenizer((char *)bL->string);
#if 0 // Print tokenized strings
        for (uint8_t i=0; i<PARSER_MAX_TOKENS; i++)
        {
            b_printf("\"%s\", \'%c\'\n",bToken->t[i].str,bToken->t[i].op);
            if (!bToken->t[i].op) break;
        }
#endif
      if (ExecLine.statement) // same line for/next implementation
         for (uint8_t s = 0; s < ExecLine.statement && bToken->t[bToken->ptr].op; s++)
            while (bToken->t[bToken->ptr++].op != ':')
               if (!bToken->t[bToken->ptr].op)
                  break;

      ExecLine.number = bL->number;
      ExecLine.nextNum = bL->next ? ((_bas_line_t *)bL->next)->number : 0;

#define CATCH_STRING 0
#if CATCH_STRING
      if (ExecLine.number == CATCH_STRING)
         asm("BKPT #0");
#endif

      if (basic_line_eval() != BASIC_STAT_OK)
      {
         switch (BasicStat)
         {
         case BASIC_STAT_ERR:
            basic_message(BASIC_MSG_ERROR, "%s, %d:%d", BErrorText[BasicError], ExecLine.number, ExecLine.statement);
            BasicStat = BASIC_STAT_OK;
            ExecLine.state = PROG_STATE_DONE;
            return BasicError = BASIC_ERR_NONE;
         case BASIC_STAT_STOP:
            basic_message(BASIC_MSG_WARNING, "Stopped, %d:%d", ExecLine.number, ExecLine.statement);
            BasicStat = BASIC_STAT_OK;
            ExecLine.state = PROG_STATE_DONE;
            return BasicError = BASIC_ERR_NONE;
         case BASIC_STAT_JUMP:
         {
            if (bL->number != ExecLine.number)
            {
               bL = BasicProg;
               while (bL && (bL->number != ExecLine.number))
                  bL = bL->next;
            }
            // ExecLine.statement = 0;
            contbL = bL;
            if (!bL)
               return BasicError = BASIC_ERR_INVALID_LINE;
         }
         break;
         case BASIC_STAT_CONT:
            ExecLine.state = PROG_STATE_RUN;
            break;
         default:
            bL = bL->next;
            ExecLine.statement = 0;
            break;
         }
      }
      else
      {
         contbL = bL;
         bL = bL ? bL->next : NULL;
         ExecLine.statement = 0;
      }
      if (keyboard_break())
      {
         ContLine = ExecLine;
         basic_message(BASIC_MSG_WARNING, "Break into program: %d:%d", ExecLine.number, ExecLine.statement);
         ExecLine.state = PROG_STATE_BREAK;
         break;
      }
      taskYIELD();
   }
   if (ExecLine.state == PROG_STATE_RUN)
      basic_message(BASIC_MSG_NORMAL, "Done, %d:%d", ExecLine.number, ExecLine.statement);
   ExecLine.state = (ExecLine.state == PROG_STATE_BREAK) ? PROG_STATE_BREAK : PROG_STATE_DONE;
   return BasicError = BASIC_ERR_NONE;
}

_bas_err_e __cont(_rpn_type_t *param)
{
   if (ExecLine.state == PROG_STATE_BREAK)
   {
      bL = contbL;
      ExecLine = ContLine;
      BasicStat = BASIC_STAT_CONT;
   }
   return BasicError = BASIC_ERR_NONE;
}
static struct
{
   char str[BASIC_STRING_LEN];
   uint8_t ptr;
} sysRetStr = {0};
void _bbuff_putc(char c)
{
   sysRetStr.str[sysRetStr.ptr] = c;
   if (sysRetStr.ptr < BASIC_STRING_LEN - 1)
      sysRetStr.ptr++;
   sysRetStr.str[sysRetStr.ptr] = '\0';
}
/**
 * @brief run interface command
 * @usage sys "iface:cmd","var1","var2",...
 * @param param
 */
_bas_err_e __sys(_rpn_type_t *param)
{
   _rpn_type_t *var;
   void *lastPutch = stdio->putch;
   sysRetStr.ptr = 0;
   *sysRetStr.str = '\0';
   char *response_token;
   // bool head = true;
   if (!param->var.i)
      return BasicError = BASIC_ERR_FEW_ARGUMENTS;
   if (token_eval_expression(param->var.i))
      return BasicError;
   var = rpn_peek_queue(true);
   if (var->type != VAR_TYPE_STRING)
      return BasicError = BASIC_ERR_TYPE_MISMATCH;
   stdio->putch = _bbuff_putc;
   exec_line(var->var.str);
   stdio->putch = lastPutch;
   // system call error process
   if (strstr(sysRetStr.str, "E:"))
   {
      b_printf("%s\n", sysRetStr.str);
      return BasicError = BASIC_ERR_SYSCALL_ERROR;
   }
   // return variables process
   response_token = strtok(sysRetStr.str, ", \n");
   while ((var = rpn_peek_queue(false))->type != VAR_TYPE_NONE)
   {
      _bas_var_t *progVar;
      if (var->type != VAR_TYPE_STRING)
         return BasicError = BASIC_ERR_TYPE_MISMATCH;
      if (!(progVar = var_get(var->var.str)))
         return BasicError = BASIC_ERR_UNKNOWN_VAR;
      if (response_token == NULL)
         return BasicError = BASIC_ERR_MANY_ARGUMENTS;
      switch (progVar->value.type)
      {
      case VAR_TYPE_FLOAT:
      case VAR_TYPE_LOOP:
         progVar->value.var.f = atof(response_token);
         break;
      case VAR_TYPE_INT:
         progVar->value.var.i = (int32_t)strtol(response_token, NULL, 0);
         break;
      case VAR_TYPE_WORD:
         progVar->value.var.i = (uint16_t)strtol(response_token, NULL, 0);
         break;
      case VAR_TYPE_BYTE:
         progVar->value.var.i = (uint8_t)strtol(response_token, NULL, 0);
         break;
      case VAR_TYPE_BOOL:
         progVar->value.var.i = strchr("0fFnN", response_token[0]) ? 0 : 1;
         break;
      case VAR_TYPE_STRING:
         if (var_set_string(progVar, response_token))
            return BasicError;
         break;
      default:
         return BasicError = BASIC_ERR_TYPE_MISMATCH;
      }
      response_token = strtok(NULL, ", \n");
   }
   return BasicError = BASIC_ERR_NONE;
}

void prog_load(char *progFileName)
{
   uint16_t procLine = 0;
   if (*progFileName && !isdigit((uint16_t)*progFileName))
   {
      _rpn_type_t fileName = {.type = VAR_TYPE_STRING, .var.str = progFileName};
      __load(&fileName);
   }
#if 0
   else /// LOAD from an array
   {
      static const _bas_rom_t *romProg[3] = {&ROM_bounce, &ROM_ctree, &ROM_snake};
      uint8_t romIndex = 0;
      if (isdigit((uint16_t)*progFileName))
         romIndex = (uint8_t)strtol(progFileName, NULL, 10);
      if (romIndex > 2)
         romIndex = 0;
      // const _bas_rom_t *romProg = &ROM_bounce;
      uint8_t *prog = (uint8_t *)romProg[romIndex]->prog;
      while (*prog && !BasicError)
      {
         while (*prog && (*prog <= ' '))
            prog++; // skip spaces and control characters
         if (!*prog)
            break; // eol/eof
         if ((ExecLine.number = (uint16_t)strtol((char *)prog, (char **)&prog, 10)) == 0)
            BasicError = BASIC_ERR_LOAD_NONUMBER;
         if (!BasicError && prog_find_line(ExecLine.number))
            BasicError = BASIC_ERR_LOAD_DUPLICATE;
         if (!BasicError && !prog_add_line(ExecLine.number, &prog))
            BasicError = BASIC_ERR_MEM_OUT;
         procLine++;
      }
      progFileName = (char *)romProg[romIndex]->name;
   }
#endif
   if (BasicError)
      b_printf("\n%s, %d:0\n", BErrorText[BasicError], procLine);
   else
      b_printf("\"%s\" loaded, %d bytes free.\n", progFileName, xPortGetFreeHeapSize());
   return;
}

void prog_run(uint16_t lineNumber)
{
   _rpn_type_t lNum = {.type = VAR_TYPE_WORD, .var.i = lineNumber};
   _terminal_t tmpTerm = uTerm;
   _stream_io_t *lastStream = stdio;
   vTaskSuspend(xuTermTask);
   taskYIELD();
   stdio = &basicStream;
   uTerm.cursorEn = false;
   ExecLine.state = PROG_STATE_NEW;
   __run(&lNum);
   taskYIELD();
   stdio = lastStream;
   tmpTerm.cursorLine = uTerm.cursorLine;
   tmpTerm.cursorCol = uTerm.cursorCol;
   uTerm = tmpTerm;
   vTaskResume(xuTermTask);
}

_bas_err_e array_set(char *name, bool init) // set array elements
{
   uint8_t bracketCnt = 1; // count square brackets
   uint8_t i;
   uint16_t dimPtr[2] = {0, 0};
   _rpn_type_t *dimVar;
   _bas_var_t *var;
   if ((var = var_get(name)) == NULL)
      return BasicError = BASIC_ERR_UNKNOWN_VAR;
   if (!init)
   {
      bToken->ptr++;
      for (i = bToken->ptr; i < PARSER_MAX_TOKENS - 1 && bToken->t[i].op; i++)
      {
         if (bToken->t[i].op == '[')
            bracketCnt++;
         if (bToken->t[i].op == ']')
            bracketCnt--;
         if (!bracketCnt)
         {
            bToken->t[i].op = ';'; // set delimeter for token evaluation
            break;
         }
      }
      if (bToken->t[i].op != ';')
         return BasicError = BASIC_ERR_PAR_MISMATCH;
      if (token_eval_expression(0) != BASIC_ERR_NONE)
         return BasicError;
      bToken->ptr++;
      if ((dimVar = rpn_pull_queue())->type == VAR_TYPE_NONE)
         return BasicError = BASIC_ERR_ARRAY_DIMENTION;
      if (var->param.size[1] &&
          (var->value.type != VAR_TYPE_ARRAY_STRING)) // two dimentional array
      {
         dimPtr[0] =
             (uint16_t)((dimVar->type & VAR_TYPE_FLOAT) ? dimVar->var.f : (dimVar->type & VAR_TYPE_INT) ? dimVar->var.i :
                                                                                                          0);
         if (dimPtr[0] >= var->param.size[1])
            return BasicError = BASIC_ERR_ARRAY_OUTOFRANGE;
         if ((dimVar = rpn_pull_queue())->type == VAR_TYPE_NONE)
            return BasicError = BASIC_ERR_ARRAY_DIMENTION;
         dimPtr[1] =
             (uint16_t)((dimVar->type & VAR_TYPE_FLOAT) ? dimVar->var.f : (dimVar->type & VAR_TYPE_INT) ? dimVar->var.i :
                                                                                                          0);
      }
      else
         dimPtr[0] =
             (uint16_t)((dimVar->type & VAR_TYPE_FLOAT) ? dimVar->var.f : (dimVar->type & VAR_TYPE_INT) ? dimVar->var.i :
                                                                                                          0);
      if (dimPtr[0] >= var->param.size[0])
         return BasicError = BASIC_ERR_ARRAY_OUTOFRANGE;
      if ((var->value.type != VAR_TYPE_ARRAY_STRING) &&
          (rpn_pull_queue()->type != VAR_TYPE_NONE))
         return BasicError = BASIC_ERR_ARRAY_DIMENTION;
   }
   if (bToken->t[bToken->ptr++].op != '=')
      return BasicError = BASIC_ERR_MISSING_EQUAL;
   if (token_eval_expression(0) != BASIC_ERR_NONE)
      return BasicError;

   bool head = true;
   uint32_t arrayPtr = dimPtr[0] + dimPtr[1] * var->param.size[1];
   uint32_t arrayLimit =
       var->param.size[0] * var->param.size[1] +
       ((var->value.type == VAR_TYPE_ARRAY_STRING) ? 0 : var->param.size[0]);
   uint8_t dSize = var->value.type == VAR_TYPE_ARRAY_STRING ? var->param.size[1] : (var->value.type == VAR_TYPE_ARRAY_BYTE ? 1 : var->value.type == VAR_TYPE_ARRAY_WORD ? 2 :
                                                                                                                                                                          4);
   void *data = var->value.var.array + arrayPtr * dSize;
   for (; arrayPtr < arrayLimit; arrayPtr++)
   {
      if ((dimVar = rpn_peek_queue(head))->type == VAR_TYPE_NONE)
         break;
      head = false;
      switch (var->value.type)
      {
      case VAR_TYPE_ARRAY_BYTE:
         *(uint8_t *)data =
             (uint8_t)(dimVar->type & VAR_TYPE_FLOAT ? dimVar->var.f : dimVar->var.i);
         break;
      case VAR_TYPE_ARRAY_WORD:
         *(uint16_t *)data =
             (uint16_t)(dimVar->type & VAR_TYPE_FLOAT ? dimVar->var.f : dimVar->var.i);
         break;
      case VAR_TYPE_ARRAY_INT:
         *(int32_t *)data = (int32_t)(dimVar->type & VAR_TYPE_FLOAT ? dimVar->var.f : dimVar->var.i);
         break;
      case VAR_TYPE_ARRAY_FLOAT:
         *(float *)data = (float)(dimVar->type & VAR_TYPE_FLOAT ? dimVar->var.f : dimVar->var.i);
         break;
      case VAR_TYPE_ARRAY_STRING:
         if (dimVar->type != VAR_TYPE_STRING)
            return BasicError = BASIC_ERR_TYPE_MISMATCH;
         else
            tstrncpy((char *)data, dimVar->var.str, var->param.size[1] - 1);
         // b_sprintf((char *)data, var->param.size[1] - 1,"%s",dimVar->var.str);
         break;
      default:
         break;
      }
      data += dSize;
   }
   if (rpn_peek_queue(false)->type != VAR_TYPE_NONE)
      return BasicError = BASIC_ERR_ARRAY_OUTOFRANGE;
   return BasicError = BASIC_ERR_NONE;
}
/*
 -mthumb -ffunction-sections -fdata-sections -mlong-calls -mcpu=cortex-m4 -std=gnu99 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-math-errno -fsingle-precision-constant -ffast-math
  -mthumb;-Wl,-Map=$(ProjectName).map --specs=nosys.specs -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group -Wl,--gc-sections -mcpu=cortex-m4;-mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-math-errno -fsingle-precision-constant -ffast-math
 * */