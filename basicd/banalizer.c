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
#include "banalizer.h"
#include "bcore.h"
#include "bfunc.h"

/***
 * https://en.wikipedia.org/wiki/Shunting_yard_algorithm
 * https://rosettacode.org/wiki/Parsing/Shunting-yard_algorithm
 */

_bas_token_t bLineTokBuff[PARSER_MAX_TOKENS];
_bas_tok_list_t bLineToken = {.t = bLineTokBuff};
_bas_tok_list_t *bToken = &bLineToken;
#define TOKEN_STACK_DEPTH 8
struct
{
   _bas_tok_list_t *tokens[TOKEN_STACK_DEPTH];
   uint8_t ptr;
} tList =
    {
        .ptr = 0,
};

const struct
{
   const uint8_t opCode;
   const uint8_t prec;
} PattOps[] =
    {
        {'(', -1},
        {'[', -1},
        {0x80, 6}, // rpn function
        {OPERATOR_MUL, 5},
        {OPERATOR_DIV, 5},
        {OPERATOR_MOD, 5},
        {OPERATOR_PLUS, 4},
        {OPERATOR_MINUS, 4},
        {OPERATOR_MORE, 3},
        {OPERATOR_LESS, 3},
        {OPERATOR_EQUAL, 3},
        {OPERATOR_MORE_EQ, 3},
        {OPERATOR_NOT_EQ, 3},
        {OPERATOR_LESS_EQ, 3},
        {OPERATOR_NOT, 2},
        {OPERATOR_BWNOT, 2},
        {OPERATOR_OR, 1},
        {OPERATOR_AND, 1},
        {OPERATOR_BWXOR, 1},
        {OPERATOR_BWOR, 1},
        {OPERATOR_BWAND, 1},
        {OPERATOR_BWSL, 1},
        {OPERATOR_BWSR, 1},
        {0}};

bool tok_list_push(_bas_tok_list_t *tokensList)
{
   if (tList.ptr >= TOKEN_STACK_DEPTH) return false;
   tList.tokens[tList.ptr++] = bToken;
   bToken = tokensList;
   return true;
}

bool tok_list_pull(void)
{
   if (tList.ptr == 0) return false;
   bToken = tList.tokens[--tList.ptr];
   return true;
}

static char get_precedence(uint8_t opCode)
{
   if (opCode > 0x80) return 6;
   if (opCode)
      for (uint8_t i = 0; i < 255 && PattOps[i].opCode; i++)
         if (PattOps[i].opCode == opCode) return PattOps[i].prec;
   return -1;
}

bool tokenizer(char *str) // return false if total number of bLineToken.ts is greater than PARSER_MAX_bToken->tS, otherwise true
{
   static char tokenString[BASIC_LINE_LEN];
   uint8_t strPtr = 0;
   uint8_t lastOp = 0;
   bool trSpace = false;
   bool tokStr = false;
   bool done = false;
   bool quoted = false;

   tstrncpy(tokenString, str, BASIC_LINE_LEN);
   str = tokenString;
   bLineToken.ptr = 0;
   bLineToken.t[bLineToken.ptr].op = 0;
   while (!done)
   {
      switch ((uint8_t)str[strPtr])
      {
      case '\r':
      case '\n':
      case '\'': // remark
         if (quoted)
         {
            strPtr++;
            break;
         }
      case '\0':
         done = true;
         quoted = false;
      case OPERATOR_MUL:
      case OPERATOR_DIV:
      case OPERATOR_MOD:
      case OPERATOR_MORE_EQ:
      case OPERATOR_NOT_EQ:
      case OPERATOR_LESS_EQ:
      case OPERATOR_AND:
      case OPERATOR_OR:
      case OPERATOR_NOT:
      case OPERATOR_BWNOT:
      case OPERATOR_BWXOR:
      case OPERATOR_BWOR:
      case OPERATOR_BWAND:
      case OPERATOR_BWSL:
      case OPERATOR_BWSR:
      case '[':
      case ']':
      case '(':
      case ')':
      case ',':
      case ':' ... '>':
         if (quoted)
            strPtr++;
         else
            bLineToken.t[bLineToken.ptr].op = done ? 0x01 : str[strPtr];
         break;
      /// --- minus and plus process
      case OPERATOR_MINUS:
      case OPERATOR_PLUS:
         if ((!done && (strPtr == 0) && (lastOp != ')' && lastOp != ']')) ||
             (quoted || (is_digit(str[strPtr - 2]) && ((str[strPtr - 1] == 'E') || (str[strPtr - 1] == 'e'))))) // || (*str & OPCODE_MASK))) // negative number or exponent sign
            strPtr++;
         else
            bLineToken.t[bLineToken.ptr].op = done ? 0x01 : str[strPtr];
         break;

      case '"': // process quoted string
         if (quoted)
         {
            if (!(strPtr && str[strPtr - 1] == '\\')) // skip if it's '\"'
            {
               quoted = false;
               strPtr++;
               break;
            }
         }
         else
            quoted = true;
      case '.':
      case '$': // string variable
      case '#': // integer variable
      case '_': // byte variable
      case '?':
      case '0' ... '9':
      case 'a' ... 'z':
      case 'A' ... 'Z':
         tokStr = true;
         if (trSpace) // second statement
            bLineToken.t[bLineToken.ptr].op = ' ';
         else strPtr++;
         break;

         // case FUNC_TYPE_SECONDARY ... 0xff:
         // case FUNC_TYPE_PRIMARY ... (FUNC_TYPE_SECONDARY-1):
      case FUNC_TYPE_PRIMARY ... 0xff:
         if (trSpace) // there is non terminated operand
         {
            bLineToken.t[bLineToken.ptr].op = ' ';
            strPtr--;
            break;
         }

         strPtr++;
         bLineToken.t[bLineToken.ptr].op = (str[strPtr] == '(' || str[strPtr] == ':') ? str[strPtr] : (str[strPtr]) ? ' ' :
                                                                                                                      '\0';
         break;
      case '\\':
         if (quoted)
            for (uint8_t tmpPtr = strPtr; (uint8_t)str[tmpPtr] > '\r'; tmpPtr++)
               str[tmpPtr] = str[tmpPtr + 1]; // skip the backslash
         strPtr++;
         break;
      default:
         if (!tokStr) str++; // remove leading spaces or unprocessed characters
         else
         {
            if (!quoted)
            {
               str[strPtr] = 0;
               trSpace = true;
            }
            strPtr++;
         }
      }
      if (bLineToken.t[bLineToken.ptr].op)
      {
         if (done) bLineToken.t[bLineToken.ptr].op = '\0';
         lastOp = bLineToken.t[bLineToken.ptr].op;
         bLineToken.t[bLineToken.ptr++].str = str;
         str[strPtr++] = 0;
         str = &str[strPtr];
         strPtr = 0;
         tokStr = trSpace = quoted = false;
         bLineToken.t[bLineToken.ptr].op = '\0';
      }
      if (bLineToken.ptr >= PARSER_MAX_TOKENS) return false;
   }
   bLineToken.ptr = 0;
   return true;
}

_bas_err_e token_eval_expression(uint8_t opParam) // if subEval is true, the will evaluate the first bracked expression, including function
{
#define RPN_PRINT_DEBUG 0
   uint8_t opCode;
   char *tokenStr;
   _bas_var_t *variable;
   bToken->parCnt = opParam == '(' ? 1 : 0;

   BasicError = BASIC_ERR_NONE;
   rpn_purge_queue();
   if ((opParam == '(') || (opParam == '!'))
      rpn_push_stack(opParam); // bracket from primary operator

   for (; bToken->ptr < PARSER_MAX_TOKENS; bToken->ptr++)
   {
      tokenStr = bToken->t[bToken->ptr].str;
      if (!tokenStr || ((*tokenStr & OPCODE_MASK) && ((uint8_t)*tokenStr < FUNC_TYPE_SECONDARY))) // primary operator found
      {
         if (tok_list_pull())
            continue; // another token list available
         else
            break; // all tokens processed
      }

      if (*tokenStr)
      {
         if (*tokenStr == '\"')
         {
            tokenStr[strlen(tokenStr) - 1] = '\0'; // remove the last quote
            rpn_push_queue(RPN_STR(tokenStr + 1)); // store without the first quote
         }
         else if (is_digit(*tokenStr) || (*tokenStr == '-') || (*tokenStr == '.')) // support numbers, negative numbers and float numbers starting with .
         {
            if ((tokenStr[1] != 'x') && (strchr(tokenStr, '.') || strchr(tokenStr, 'E') || strchr(tokenStr, 'e')))
               rpn_push_queue(RPN_FLOAT(atof(tokenStr)));
            else
               rpn_push_queue(RPN_INT(tokenStr[1] == 'b' ? strtol((char *)(tokenStr + 2), NULL, 2) : strtol(tokenStr, NULL, 0)));
         }
         else
         {
            if ((variable = var_get(bToken->t[bToken->ptr].str)) != NULL)
            {
               if (variable->value.type & VAR_TYPE_ARRAY) // array
               {
                  if (bToken->t[bToken->ptr].op != '[') return BasicError = BASIC_ERR_PAR_MISMATCH;
                  if (rpn_push_stack(__OPCODE_ARRAY) != BASIC_ERR_NONE) return BasicError;
                  if (rpn_push_stack('[') != BASIC_ERR_NONE) return BasicError;
                  if (rpn_push_queue((_rpn_type_t){
                          .type = VAR_TYPE_ARRAY, .var.array = variable}) != BASIC_ERR_NONE) return BasicError;
                  bToken->parCnt++;
                  continue;
               }
               else if (variable->value.type & VAR_TYPE_DEFFN) // function
               {
                  if (bToken->t[bToken->ptr].op != '(') return BasicError = BASIC_ERR_PAR_MISMATCH;
                  if (rpn_push_stack(__OPCODE_DEFFN) != BASIC_ERR_NONE) return BasicError;
                  if (rpn_push_stack('(') != BASIC_ERR_NONE) return BasicError;
                  if (rpn_push_queue((_rpn_type_t){
                          .type = VAR_TYPE_DEFFN, .var.deffn = variable}) != BASIC_ERR_NONE) return BasicError;
                  bToken->parCnt++;
                  continue;
               }
               else
                  rpn_push_queue(variable->value);
            }
            else
            {
               opCode = (uint8_t)*tokenStr;
               if ((opCode & OPCODE_MASK) && (opCode < __OPCODE_LAST)) // check that the opcode is valid
               {
                  if (rpn_push_stack((uint8_t)*tokenStr) != BASIC_ERR_NONE) return BasicError;
               }
               else
                  return BasicError = BASIC_ERR_UNKNOWN_VAR;
            }
         }
         //  bToken->ptr++;
      }
      if ((!bToken->t[bToken->ptr].op) || (bToken->t[bToken->ptr].op == ':') || (bToken->t[bToken->ptr].op == ';')) /// eol, ":" or terminator ends the evaluation
      {
         if (tok_list_pull())
            continue; // another token list available
         else
            break; // all tokens processed
      }
      switch (bToken->t[bToken->ptr].op)
      {
      case ' ': // skip
         break;
      case ',':
         if (bToken->parCnt) // evaluate inside the brackets
         {
            while (rpn_peek_stack_last() != '(' && rpn_peek_stack_last() != '[')
               if (rpn_eval(rpn_pull_stack()) != BASIC_ERR_NONE) return BasicError;
         }
         else // evaluate the stack and store in the queue
         {
            while ((opCode = rpn_pull_stack())) // && (opCode != '(') && (opCode != '['))
               if (rpn_eval(opCode) != BASIC_ERR_NONE) return BasicError;
         }
         break;
      case '(':
         bToken->parCnt++;
         rpn_push_stack(bToken->t[bToken->ptr].op);
         break;
      case ')':
      case ']':
         if (bToken->parCnt) bToken->parCnt--;
         else
            return BasicError = BASIC_ERR_PAR_MISMATCH;
         while (rpn_peek_stack_last() != '(' && rpn_peek_stack_last() != '[')
         {
            if (!rpn_peek_stack_last()) return BasicError = BASIC_ERR_PAR_MISMATCH;
            if (rpn_eval(rpn_pull_stack()) != BASIC_ERR_NONE) break;
         }
         rpn_pull_stack(); // remove the opening bracket
         if (rpn_peek_stack_last() > OPCODE_MASK)
            if (rpn_eval(rpn_pull_stack()) != BASIC_ERR_NONE) return BasicError; // evaluate function
         break;
      case '^': // ^ is evaluated right-to-left, natively to RPN, so just stack it
         rpn_push_stack(bToken->t[bToken->ptr].op);
         break;
      default:
         if (get_precedence(bToken->t[bToken->ptr].op) > get_precedence(rpn_peek_stack_last())) rpn_push_stack(bToken->t[bToken->ptr].op);
         else
         {
            while ((rpn_peek_stack_last() != '(' && rpn_peek_stack_last() != '[') && (get_precedence(bToken->t[bToken->ptr].op) <= get_precedence(rpn_peek_stack_last())))
               if (rpn_eval(rpn_pull_stack()) != BASIC_ERR_NONE) break;
            rpn_push_stack(bToken->t[bToken->ptr].op);
         }
      }
#if RPN_PRINT_DEBUG
      rpn_print_queue(true);
      rpn_print_stack(true);
      if (BasicError) b_printf("RPN error(1): %s\n", BErrorText[BasicError]);
#endif
      if (BasicError) return BasicError;
      taskYIELD();
   }
   if (bToken->parCnt) return BasicError = BASIC_ERR_PAR_MISMATCH;
   while ((opCode = rpn_pull_stack()))
   {
      rpn_eval(opCode);
      taskYIELD();
#if RPN_PRINT_DEBUG
      rpn_print_queue(true);
      rpn_print_stack(true);
      if (BasicError) b_printf("RPN error(2): %s\n", BErrorText[BasicError]);
#endif
      if (BasicError) return BasicError;
   }
   return BasicError; // = bToken->parCnt ? BASIC_ERR_PAR_MISMATCH : BASIC_ERR_NONE;
}
