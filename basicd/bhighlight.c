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
// #include "banalizer.h"
#include "bcore.h"
// #include "bfunc.h"
#include "bsp.h"
#include "freeRTOS.h"
#include "lcd.h"
// #include "task.h"
#include "bfunc.h"
#include "bhighlight.h"
#include "tstring.h"
#include "uterm.h"

const uint8_t SyntaxHLdark[SYNCOL_COUNT] = {
    _rgb(0x00, 0x00, 0x00), // SYNCOL_BACKGROUND,
    _rgb(0xc0, 0xd0, 0xd0), // SYNCOL_DEFAULT,
    _rgb(0xe0, 0xc0, 0x20), // SYNCOL_WARNING,
    _rgb(0xc0, 0x20, 0x20), // SYNCOL_ERROR,
    _rgb(0x40, 0x40, 0x40), // SYNCOL_COMMENT,
    _rgb(0xe0, 0xc0, 0x80), // SYNCOL_DELIMITER,
    _rgb(0xc0, 0x00, 0x00), // SYNCOL_BRACKET,
    _rgb(0x60, 0xe0, 0x40), // SYNCOL_LINE_NUM,
    _rgb(0xc0, 0x40, 0x80), // SYNCOL_NUMBER,
    _rgb(0x50, 0x50, 0xf0), // SYNCOL_STRING,
    _rgb(0xf0, 0x70, 0x70), // SYNCOL_OPERATOR,
    //_rgb(0xa0, 0x00, 0xf0), // SYNCOL_FUNC_PRIMARY,
    _rgb(0x60, 0x60, 0xc0), // SYNCOL_CONTROL, : if then for to step next goto gosub
    _rgb(0xa0, 0xa0, 0x00), // SYNCOL_FUNC_PRIMARY,
    _rgb(0x70, 0xe0, 0xf0), // SYNCOL_FUNC_SECONDARY,
};

uint8_t *HLScheme = (uint8_t *)SyntaxHLdark;
char *Operators[] = {"and", "or", "not", ""};
char tmpTok[16];
bool synLineNumber;

uint8_t tok_colour(char *str)
{
  uint8_t colour = SYNCOL_DEFAULT;
  uint8_t length = 0;
  bool done = false;
  bool number = false;
  while (*str)
    {
      switch (*str)
        {
        case '\'': // comment processing
          if (!length)
            {
              colour = SYNCOL_COMMENT;
              while (*str++)
                length++;
            }
          done = true;
          break;
        case '\"': // string processing
          if (!length)
            {
              char cc = '\\';
              colour = SYNCOL_STRING;
              while (*str)
                {
                  length++;
                  if ((*str == '\"') && (cc != '\\'))
                    break;
                  cc = *str;
                  str++;
                }
            }
          done = true;
          break;
        case ',':
        case ';':
        case ':':
        case ' ':
          if (!length)
            {
              colour = SYNCOL_DELIMITER;
              length++;
            }
          done = true;
          break;
        case ']':
        case '[':
        case '(':
        case ')':
          if (!length)
            {
              colour = SYNCOL_BRACKET;
              length++;
            }
          done = true;
          break;
        case '+':
        case '-':
        case '/':
        case '*':
        case '%':
        case '>':
        case '<':
        case '=':
        case '!':
          if (!length)
            {
              colour = SYNCOL_OPERATOR;
              length++;
            }
          done = true;
          break;
        case '0' ... '9':
          if (!length)
            number = true;
          colour = SYNCOL_NUMBER;
          break;
        default:
          break;
        }
      if (done || (length > (sizeof(tmpTok) - 2)))
        break;
      tmpTok[length++] = *str++;
    }
  if (number)
    colour = synLineNumber ? SYNCOL_LINE_NUM : SYNCOL_NUMBER;
  synLineNumber = false;
  if (length && (colour == SYNCOL_DEFAULT))
    {
      tmpTok[length] = 0;
      uint8_t opCode;
      for (uint8_t i = 0; i < 3; i++)
        {
          if (!strcmp(Operators[i], tmpTok))
            {
              uTerm.fgColour = HLScheme[SYNCOL_OPERATOR];
              return length;
            }
        }
      if ((opCode = bas_func_opcode(tmpTok)))
        {
          uTerm.fgColour = HLScheme[opCode < FUNC_TYPE_SECONDARY ? (opCode < FUNC_TYPE_PROCEDURE ? SYNCOL_CONTROL : SYNCOL_FUNC_PRIMARY) : SYNCOL_FUNC_SECONDARY];
          return length;
        }
    }

  uTerm.fgColour = HLScheme[colour];
  return length;
}

void highlight(char *str)
{
  uint8_t lastFGColour = uTerm.fgColour;
  // uint8_t lastBGColour = uTerm.bgColour;
  synLineNumber = true;
  while (*str && (*str != '\n'))
    {
      uint8_t len = tok_colour(str);
      for (uint8_t i = 0; i < len; i++)
        stdio->putch(*str++);
    }
  uTerm.fgColour = lastFGColour;
  // uTerm.bgColour = lastBGColour;
}
