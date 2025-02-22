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
#include "banalizer.h"
#include "bcore.h"
#include "bhighlight.h"
#include "bsp.h"
#include "bstring.h"
#include "editline.h"
#include "freeRTOS.h"
#include "keyboard.h"
#include "task.h"
#include "uterm.h"
#include "bedit.h"
#include <stdlib.h>

static char bclString[BASIC_LINE_LEN+6];
_editline_t basicLine =
    {
        .str = bclString,
        .curPos = 0,
        .length = 0,
        .maxLen = BASIC_LINE_LEN+6
    };

void basic_putch(char cc);
_stream_io_t basicStream =
    {
        .putch = basic_putch,
        .getch = keyboard_getch,
};

void basic_message(enum _bas_msg_e type,const char *str, ... )
{
    va_list arg;
    va_start(arg,str);
    if(uTerm.cursorCol)
        basic_putch('\n');
    switch(type)
    {
        case   BASIC_MSG_NORMAL:  
            uTerm.fgColour = HLScheme[SYNCOL_DEFAULT];
            break;
        case   BASIC_MSG_WARNING:
            uTerm.fgColour = HLScheme[SYNCOL_WARNING];
            break;
        case   BASIC_MSG_ERROR:
            uTerm.fgColour = HLScheme[SYNCOL_ERROR];
            break;
    }
    tformat(stdio,str,&arg);
    va_end(arg);
    uTerm.fgColour = HLScheme[SYNCOL_DEFAULT];
    basic_putch('\n');
}

char *get_basic_line(char *str, bool syntaxHl)
{
  char cc;
  uint8_t startCol = uTerm.cursorCol, startLine = uTerm.cursorLine;
  TickType_t blinkTime;
  _ed_stat_t edStat;
  editline_set(&basicLine, str);
  blinkTime = xTaskGetTickCount();
  while (1)
    {
      taskYIELD();
      if (stdio->getch(&cc)) // if char been received
        {
          switch (edStat = editline(&basicLine, cc))
            {
            case ED_DELETE:
            case ED_BACKSPACE:
              glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(0)); // redraw current char, clear the cursor
            case ED_CHAR:
              uTerm.cursorCol = startCol;
              uTerm.cursorLine = startLine;
              if (syntaxHl)
                highlight(basicLine.str);
              else
                b_printf(basicLine.str);
              if (edStat == ED_BACKSPACE || edStat == ED_DELETE) stdio->putch(' ');// remove the trailing char
              break;

            case ED_ENTER:
              glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(0)); // redraw current char, clear the cursor
              if (*basicLine.str)
                  {
                      uTerm.cursorCol = startCol;
                      uTerm.cursorLine = startLine;
                     if (syntaxHl)
                        highlight(basicLine.str);
                     else
                        b_printf(basicLine.str);
                      b_printf("\n");
                      strncat(basicLine.str,"\n",BASIC_LINE_LEN-1);
                  }
              return basicLine.str;

            case ED_RIGHT:
              if (basicLine.curPos < basicLine.length)
                {
                  glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(0)); // redraw current char, clear the cursor
                  basicLine.curPos++;
                }
              break;
            case ED_LEFT:
              if (basicLine.curPos)
                {
                  basicLine.curPos--;
                  glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(0)); // redraw current char, clear the cursor
                }
              break;

            case ED_HOME:
              if (basicLine.curPos)
                {
                  glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(0)); // redraw current char, clear the cursor
                  basicLine.curPos = 0;
                }
              break;

            case ED_END:
              if (basicLine.curPos < basicLine.length)
                {
                  basicLine.curPos = basicLine.length;
                }
              break;

            case ED_UP: 
                uTerm.cursorCol = startCol;
                uTerm.cursorLine = startLine;
                for(uint8_t i=0;i<strlen(basicLine.str);i++) stdio->putch(' '); // clear command line
                if(BasicLineZero) strcpy(basicLine.str,(char *)basic_line_totext(BasicLineZero->string));
                else basicLine.str[0] = 0;
                uTerm.cursorCol = startCol;
                uTerm.cursorLine = startLine;
                highlight(basicLine.str);
                basicLine.curPos = basicLine.length = strlen(basicLine.str);
              break;

            case ED_DOWN: // Get line from program to edit
              if(is_digit(*basicLine.str))
              {
                  uint16_t lNum = strtol(basicLine.str, NULL, 10);
                  _bas_line_t *pLine = prog_find_line(lNum);
                  if (pLine == NULL) break;
                uTerm.cursorCol = startCol;
                uTerm.cursorLine = startLine;
                for(uint8_t i=0;i<strlen(basicLine.str);i++) stdio->putch(' '); // clear command line
                uTerm.cursorCol = startCol;
                uTerm.cursorLine = startLine;
                tsprintf(basicLine.str,"%d %s",lNum,(char *)basic_line_totext(pLine->string));
                highlight(basicLine.str);
                basicLine.curPos = basicLine.length = strlen(basicLine.str);
              }
              break;
            case ED_IN_PROCESS:
            case ED_PGUP:
            case ED_PGDOWN:
            case ED_BREAK:
            case ED_ESCAPE:
              break;
            }
        }
        uTerm.cursorCol = startCol + (basicLine.curPos % uTerm.cols);
        uTerm.cursorLine = startLine + (basicLine.curPos / uTerm.cols);        
      if (xTaskGetTickCount() - blinkTime >= TERM_BLINK_PERIOD)
        {
          blinkTime = xTaskGetTickCount();
          cursor_invert();
        }
    }
}

const _rpn_type_t ParamZero = {.var.i = 0};
const _rpn_type_t ParamByteZero = {.type = VAR_TYPE_BYTE,.var.i = 0};
cmd_err_t basic_exe(_cl_param_t *sParam)
{
  _terminal_t tmpTerm = uTerm;
  uint16_t lineNumber;
  _stream_io_t *lastStream = stdio;
  bool done = false;
  char *bLineStr;
  /// Take control of the system IO stream and save the system evironment
  vTaskSuspend(xuTermTask);
  vTaskDelay(50);
  stdio = &basicStream;
  uTerm.cursorSize = uTerm.font->height;
  uTerm.bgColour = HLScheme[SYNCOL_BACKGROUND];
  uTerm.fgColour = HLScheme[SYNCOL_DEFAULT];
  text_cls();
  BasicLineZero = pvPortMalloc(sizeof(_bas_line_t));
  memset(BasicLineZero,0x00,sizeof(_bas_line_t));
  b_printf("Basic D\n Version 0.2b\n");
  b_printf("Free mem: %d bytes\n\n", xPortGetFreeHeapSize());
  while (!done)
    {
      get_basic_line("", true);
      if(!*basicLine.str) continue; // empty line, nothing to do
      if(!strcmp("exit\n",basicLine.str)) break;
      bLineStr = basicLine.str;
      lineNumber = is_digit(*bLineStr) ? (uint16_t)strtol(bLineStr, &bLineStr, 10) : 0;
      prog_add_line(lineNumber, (uint8_t **)&bLineStr);
      if (!lineNumber)
      {
        //ExecLine.state = PROG_STATE_CMDLINE;
        __run((_rpn_type_t *)&ParamByteZero);
      }
    }
  stdio = lastStream;
  uTerm = tmpTerm;
  vTaskResume(xuTermTask);
  text_cls();
  return CMD_NO_ERR;
}