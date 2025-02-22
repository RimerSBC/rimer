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
#include "FreeRTOS.h"
#include <stdlib.h>
#include "bsp.h"
#include "keyboard.h"
#include "colours.h"
#include "rshell.h"
#include "commandline.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include "zx80sys.h"
#include "zxscreen.h"
#include "z80mnx.h"

struct
{
   struct
   {
      uint8_t height;
      uint8_t width;
      uint8_t fg;
      uint8_t bg;
   } code;
   struct
   {
      uint8_t height;
      uint8_t width;
      uint8_t fg;
      uint8_t bg;
   } regs;
   struct
   {
      uint8_t height;
      uint8_t width;
      uint8_t fg;
      uint8_t bg;
   } dump;
   struct
   {
      uint8_t fg;
      uint8_t bg;
   } edit;
   uint8_t bpFG;
   uint8_t bpBG;
   uint8_t hlBG;
   uint8_t hlFG;
   uint8_t flagsHL;
   uint8_t ptrChar;
   uint8_t bpChar;
   uint8_t findChar;
} layout;

struct
{
   uint16_t dumpAddress;
   uint16_t lookUpAddress;
   uint16_t lookUpDispAddr;
   uint16_t breakPoint;
   bool focusCode;
} zdbState =
    {
        .dumpAddress = 0x1538, // Pints to "(C) 1982 Sinclair Research Ltd." message
        .lookUpAddress = 0x0000,
        .lookUpDispAddr = 0x0000,
        .breakPoint = 0xffff, // disabled
        .focusCode = true,
};

char intrStr[32];
static Z80_STATE lastState = {0};
uint16_t *lineDispAddr; /// list of displayed address

_stream_io_t *ucosRStream;

static const char *HelpText[] =
    {
        "    ** Keystrokes help **",
        " ",
        " \'H\' - this help",
        " \'C\' - continue",
        " \'D\' - dump memory address",
        " \'G\' - go to address and mark",
        " \'M\' - set mark at PC",
        " \'6\' - move mark down",
        " \'7\' - move mark up",
        " \'B\' - toggle break point",
        " \'S\' - show break point",
        " \'R\' - run to break point",
        " \'V\' - view zx screen",
        " \'CS+Break' - Exit",
        " ",
        "  -- Press a key to exit --",
        "",
};

static void zgdb_putch(char cc)
{
   if (cc < ' ')
      return;
   glyph_xy(uTerm.cursorCol, uTerm.cursorLine, glyphChar(cc));
   if (uTerm.cursorCol < uTerm.cols - 1)
      uTerm.cursorCol++;
}

_stream_io_t zgdbStream =
    {
        .putch = zgdb_putch,
        .getch = keyboard_getch,
};

void text_xy(uint8_t line, uint8_t col)
{
   uTerm.cursorLine = line;
   uTerm.cursorCol = col;
}

void text_colour(uint8_t foreground, uint8_t background)
{
   uTerm.fgColour = foreground;
   uTerm.bgColour = background;
}

static void help(void)
{
   text_colour(layout.code.fg, layout.code.bg);
   for (uint8_t l = 0; l < layout.code.height; l++)
   {
      text_xy(l, 0);
      for (uint8_t c = 0; c < layout.code.width; c++)
         stdio->putch(' ');
   }
   for (uint8_t l = 0; l < layout.code.height && *HelpText[l]; l++)
   {
      text_xy(l, 0);
      tprintf(HelpText[l]);
   }
}

static void zdb_init(void)
{
   static bool initialized = false;
   if (initialized)
      return;

   /// Layout dump area params
   layout.dump.height = 4;
   layout.dump.width = uTerm.cols; //40;
   layout.dump.fg = _rgb(0x00, 0x80, 0x40);
   layout.dump.bg = ANSI_pal256[SC_BLACK];
   /// Layout regiser area params
   layout.regs.height = uTerm.lines - layout.dump.height;//16;
   layout.regs.width = 10;
   layout.regs.fg = ANSI_pal256[SC_WHITE];
   layout.regs.bg = ANSI_pal256[SC_BLUE];
   /// Layout code area params
   layout.code.height = layout.regs.height; //16;
   layout.code.width = uTerm.cols - layout.regs.width; // 30;
   layout.code.fg = ANSI_pal256[SC_BLACK];
   layout.code.bg = ANSI_pal256[SC_WHITE];
   /// Layout edit area params
   layout.edit.fg = ANSI_pal256[SC_BLUE];
   layout.edit.bg = ANSI_pal256[SC_BRIGHT_WHITE];
   /// Layout common params
   layout.bpFG = ANSI_pal256[SC_BRIGHT_YELLOW];
   layout.bpBG = ANSI_pal256[SC_MAGENTA];
   layout.hlFG = ANSI_pal256[SC_BRIGHT_YELLOW];
   layout.hlBG = ANSI_pal256[SC_BRIGHT_WHITE];
   layout.flagsHL = ANSI_pal256[SC_BRIGHT_MAGENTA];
   layout.ptrChar = '>';
   layout.bpChar = '@';
   layout.findChar = '=';

   lineDispAddr = pvPortMalloc(layout.code.width * sizeof(uint16_t));
}

uint8_t zdb_line(uint16_t addr, bool print)
{
   uint8_t opCode = z80mem[addr]; // opcode is valid for basic group only.
   int8_t i, byte = 0;
   uint16_t word = 0, wordH = 0;
   char *mnxPtr;
   enum _opGroup_e group = (enum _opGroup_e)(z80instr[0][opCode].group);
   switch (opCode)
   {
   case 0xCB:
      group = OPGR_CB;
      opCode = z80mem[addr + 1];
      break;
   case 0xDD:
      group = (z80mem[addr + 1] == 0xCB) ? OPGR_DDCB : OPGR_DD;
      opCode = z80mem[addr + (group == OPGR_DD ? 1 : 3)];
      break;
   case 0xED:
      group = OPGR_ED;
      opCode = z80mem[addr + 1];
      break;
   case 0xFD:
      group = (z80mem[addr + 1] == 0xCB) ? OPGR_FDCB : OPGR_FD;
      opCode = z80mem[addr + (group == OPGR_FD ? 1 : 3)];
      break;
   default:
      group = OPGR_BASIC;
   }

   if (group != OPGR_BASIC) // look for the opcode in the list
   {
      for (i = 0; z80instr[group][i].size; i++)
         if (z80instr[group][i].code == opCode)
         {
            opCode = i;
            break;
         }
      if (!z80instr[group][i].size)
         opCode = i; // opcode is not found
   }
   if (!print) // skipping print
      return z80instr[group][opCode].size ? z80instr[group][opCode].size : 1;
   mnxPtr = (char *)z80instr[group][opCode].mnmx;
   tsprintf(intrStr, " %4x:", addr);
   while (*mnxPtr)
   {
      if (*mnxPtr == '+') // process IX/IY +-
      {
         byte = (int8_t)z80mem[addr + 2];
         tsprintf(intrStr, "%s%s%d", intrStr, byte < 0 ? "" : "+", byte);
         mnxPtr += 3;
         continue;
      }
      if (*mnxPtr == '%') // process numeric data
      {
         switch (*++mnxPtr)
         {
         case 'R':
            byte = (int8_t)z80mem[addr + 1] + z80instr[group][opCode].size;
            tsprintf(intrStr, "%s%d", intrStr, byte);
            wordH = (uint16_t)(addr + byte);
            break;
         case 'B':
            byte = z80mem[addr + (group == OPGR_BASIC ? 1 : 3)];
            tsprintf(intrStr, "%s$%2x", intrStr, (uint8_t)byte, (uint8_t)byte);
            break;
         case 'A': /// Address
         case 'W': /// Word
            word = z80mem[addr + (group == OPGR_BASIC ? 2 : 4)];
            word = (word << 8) + z80mem[addr + (group == OPGR_BASIC ? 1 : 3)];
            tsprintf(intrStr, "%s$%4x", intrStr, word);
            break;
         }
         mnxPtr++;
         continue;
      }
      tsprintf(intrStr, "%s%c", intrStr, *mnxPtr++);
   }
   if (word > 9)
      tsprintf(intrStr, "%s ; %d", intrStr, word);
   else if (wordH > 9)
      tsprintf(intrStr, "%s ; $%4x", intrStr, wordH);
   else if (byte != 0)
      tsprintf(intrStr, "%s ; %d", intrStr, (uint8_t)byte);

   tprintf(intrStr);
   for (uint8_t i = 0; i < layout.code.width - strlen(intrStr); i++)
      stdio->putch(' ');
   return z80instr[group][opCode].size ? z80instr[group][opCode].size : 1;
}

enum redraw_type_e
{
   REDRAW_NONE,   // move cursor only
   REDRAW_UPDATE, // redraw current state
   REDRAW_CODE,   // redraw from PC down
   REDRAW_LOOKUP  // redraw from look-up address
};

static void code_update(enum redraw_type_e rType)
{
   bool redraw = true;
   uint8_t l; //, markLine = 0;
   uint16_t addr;
   static uint16_t lastAddr = 0;
   switch (rType)
   {
   case REDRAW_NONE:
      for (l = 0; l < layout.code.height && redraw; l++)
         if (lineDispAddr[l] == z80state.pc)
         {
            redraw = false; // current address is already on the screen
            break;
         }
      addr = z80state.pc;
      break;
   case REDRAW_UPDATE:
      addr = lineDispAddr[0];
      break;
   case REDRAW_CODE:
      addr = z80state.pc;
      break;
   case REDRAW_LOOKUP:
      addr = zdbState.lookUpDispAddr;
      break;
   }

   for (l = 0; l < layout.code.height; l++)
   {
      bool bp;
      text_xy(l, 0);
      if (redraw)
      {
         bp = (addr == zdbState.breakPoint) ? true : false;
         text_colour(bp ? layout.bpFG : layout.code.fg, bp ? layout.bpBG : (addr == zdbState.lookUpAddress) ? layout.hlBG :
                                                                                                              layout.code.bg);
         lineDispAddr[l] = addr;
         addr += zdb_line(addr, true);
         text_xy(l, 0);
         stdio->putch(lineDispAddr[l] == z80state.pc ? layout.ptrChar : ' ');
      }
      else
      {
         bp = (lineDispAddr[l] == zdbState.breakPoint) ? true : false;
         text_colour(bp ? layout.bpFG : layout.code.fg, bp ? layout.bpBG : (lineDispAddr[l] == zdbState.lookUpAddress) ? layout.hlBG :
                                                                                                                         layout.code.bg);
         if (lineDispAddr[l] == z80state.pc)
            stdio->putch(layout.ptrChar);
         else if (lineDispAddr[l] == lastAddr)
            stdio->putch(' ');
      }
   }
   lastAddr = z80state.pc;
}

static const char *regNames[15] =
    {" AF :", " BC :", " DE :", " HL :", " IX :", " IY :", " SP :", " AF':", " BC':", " DE':", " HL':", " PC :", " I:00 IM0 ", " IFF 0 0  ", " R:00     "};
static const char flagsN[8] = {'S', 'Z', '_', 'H', '_', 'P', 'N', 'C'}; // use these for undocumented: {'S', 'Z', 'Y', 'H', 'X', 'P', 'N', 'C'};

static void regs_update(bool all)
{
   uint16_t *reg = (uint16_t *)&z80state;

   uint8_t l;
   uint16_t *lastReg = (uint16_t *)&lastState;
   uint8_t flags = lastState.registers.byte[Z80_F] ^= z80state.registers.byte[Z80_F];
   text_colour(layout.regs.fg, layout.regs.bg);
   if (all)
      for (uint8_t l = 0; l < uTerm.lines-5; l++)
      {
         text_xy(l, layout.code.width);
         tprintf(l<15 ? regNames[l] : "          ");
         flags = true;
      }
   for (l = 0; l < 12; l++)
   {
      text_xy(l, layout.code.width + 5);
      text_colour((*reg == *lastReg) ? layout.regs.fg : layout.hlFG, layout.regs.bg);
      tprintf("%4x ", *reg);
      *lastReg++ = *reg++;
   }
   text_xy(l, layout.code.width + 3);
   text_colour((lastState.i != z80state.i) ? layout.hlFG : layout.regs.fg, layout.regs.bg);
   tprintf("%2x", z80state.i);
   text_xy(l++, layout.code.width + 8);
   text_colour((lastState.im != z80state.im) ? layout.hlFG : layout.regs.fg, layout.regs.bg);
   tprintf("%1x", z80state.im);
   text_xy(l++, layout.code.width + 5);
   text_colour(layout.regs.fg, layout.regs.bg);
   tprintf("%d %d", z80state.iff1, z80state.iff2);
   text_xy(l, layout.code.width + 3);
   // text_colour(layout.hlFG, layout.regs.bg);
   tprintf("%2x", z80state.r);

   /// print flags
   {
      text_xy(layout.regs.height - 1, layout.code.width);
      stdio->putch(' ');
      for (uint8_t i = 0, b = 0x80; i < 8; i++, b >>= 1)
      {
         text_colour((flags & b) ? layout.flagsHL : layout.regs.fg, layout.regs.bg);
         tprintf("%c", z80state.registers.byte[Z80_F] & b ? flagsN[i] : '_');
      }
      stdio->putch(' ');
   }
   memcpy(&lastState, &z80state, sizeof(lastState));
}

static void _z80_dump(uint16_t addr)
{
   static char ascii[9] = {0};
   uint8_t dd;
   text_colour(layout.dump.fg, layout.dump.bg);
   for (uint8_t l = 0; l < 4; l++)
   {
      text_xy(layout.regs.height + l, 0);
      tprintf("%4x:", addr);
      for (uint8_t b = 0; b < 8; b++)
      {
         dd = z80mem[addr++];
         tprintf(" %2x", dd);
         ascii[b] = ((dd > 31) && (dd < 127)) ? dd : '.';
      }
      tprintf(" |%s ", ascii);
      if (uTerm.cols > 40)
          for(uint8_t i=0;i<uTerm.cols-40;i++) stdio->putch(' ');
   }
}

static bool read_address(const char *typeName, uint16_t *value)
{
   _stream_io_t *tmpStream = stdio;
   bool success;

   text_colour(layout.edit.fg, layout.edit.bg);
   for (uint8_t l = layout.regs.height; l < layout.regs.height + 4; l++)
   {
      text_xy(l, 0);
      for (uint8_t c = 0; c < uTerm.cols; c++)
         stdio->putch(' ');
   }
   text_xy(layout.regs.height + 2, 1);
   stdio = ucosRStream;
   tsprintf(intrStr, "0x%4x", *value);
   tprintf("%s address: ", typeName);
   uTerm.cursorSize = 12;
   if ((success = edit_string(intrStr, 8, stdio)))
      *value = (uint16_t)strtol(intrStr, NULL, 0);
   flush_stream();
   uTerm.cursorSize = 0;
   stdio = tmpStream;
   _z80_dump(zdbState.dumpAddress);
   return success;
}

static uint16_t find_address(uint16_t addrToFind)
{
#define TRY_TO_ALIGN 0
#if TRY_TO_ALIGN
   uint32_t a = 0;
   if (addrToFind < lineDispAddr[0]) /// address is lower than shown ones
   {
      for (a = 0; a < lineDispAddr[0]; a += zdb_line(a, false))
         if (a >= addrToFind)
            return a;
   }

   if (addrToFind > lineDispAddr[layout.code.height - 1]) /// address is higher than shown ones
   {
      for (a = lineDispAddr[layout.code.height - 1]; a < 0xffff; a += zdb_line(a, false))
         if (a >= addrToFind)
            return a;
   }
#else
   if ((addrToFind < lineDispAddr[0]) || (addrToFind > lineDispAddr[layout.code.height - 1]))
      return addrToFind;
#endif
   for (uint8_t i = 0; i < layout.code.height; i++)
      if (lineDispAddr[i] >= addrToFind)
         return lineDispAddr[i];
   return addrToFind;
}

void zdb_process(uint16_t addr)
{
   char c;
   uint8_t l;
   enum redraw_type_e forceRedraw = REDRAW_NONE;
   while (1)
   {
      if (keyboard_break())
         return;
      if (keyboard_getch(&c))
      {
         switch (c)
         {
         case 'c':
            z80_cycle();
            forceRedraw = REDRAW_NONE;
            break;
         case 'd':
            read_address("Dump", &zdbState.dumpAddress);
            break;
         case 'h': // show help
            help();
            vTaskDelay(300);
            while (!keyboard_pressed())
               taskYIELD();
            forceRedraw = REDRAW_UPDATE;
            break;
         case 'g': /// Go to address
            if (!read_address("Go to", &zdbState.lookUpAddress))
               break;
            zdbState.lookUpAddress = (find_address(zdbState.lookUpAddress));
            zdbState.lookUpDispAddr = zdbState.lookUpAddress;
            for (l = 0; l < layout.code.height; l++)
               if (zdbState.lookUpAddress == lineDispAddr[l])
                  break;
            if (l < layout.code.height) /// address is already on the screen
               forceRedraw = REDRAW_UPDATE;
            else
               forceRedraw = REDRAW_LOOKUP;
            break;
         case 'm': /// mark on PC address
            zdbState.lookUpAddress = z80state.pc;
            for (l = 0; l < layout.code.height; l++)
               if (zdbState.lookUpAddress == lineDispAddr[l])
                  break;
            if (l < layout.code.height) /// address is already on the screen
               forceRedraw = REDRAW_UPDATE;
            else
               forceRedraw = REDRAW_CODE;
            break;
         case 'b':                                             /// set breakpoint
            if (zdbState.breakPoint == zdbState.lookUpAddress) // disable breakpoint
            {
               zdbState.breakPoint = 0xffff;
               forceRedraw = REDRAW_UPDATE;
               break;
            }

            for (l = 0; l < layout.code.height; l++)
               if (zdbState.lookUpAddress == lineDispAddr[l])
                  break;
            if (l < layout.code.height)
               zdbState.breakPoint = zdbState.lookUpAddress;
            else
               zdbState.breakPoint = zdbState.lookUpAddress = z80state.pc;

            forceRedraw = REDRAW_UPDATE;
            break;
         case 's':                             /// show breakpoint
            if (zdbState.breakPoint == 0xffff) // not set
               break;
            for (l = 0; l < layout.code.height; l++)
               if (zdbState.breakPoint == lineDispAddr[l])
                  break;
            if (l < layout.code.height)
               break; // already shown
            zdbState.lookUpDispAddr = zdbState.lookUpAddress = zdbState.breakPoint;
            forceRedraw = REDRAW_LOOKUP;
            break;
         case '6': /// move mark down
            for (l = 0; l < layout.code.height; l++)
               if (zdbState.lookUpAddress == lineDispAddr[l])
                  break;
            if (l >= layout.code.height - 1) /// already at the bottom line or not found
               break;
            zdbState.lookUpAddress = lineDispAddr[l + 1];
            forceRedraw = REDRAW_UPDATE;
            break;
         case '7': /// move mark up
            for (l = 0; l < layout.code.height; l++)
               if (zdbState.lookUpAddress == lineDispAddr[l])
                  break;
            if (!l || (l == layout.code.height)) /// at top or not found
               break;
            zdbState.lookUpAddress = lineDispAddr[l - 1];
            forceRedraw = REDRAW_UPDATE;
            break;
         case 'r': /// real time run to break point or
            vTaskSuspend(xuTermTask);
            if (zdbState.breakPoint == 0xffff) /// if no break point set
               zdbState.breakPoint = 0x11b7;   /// set break point on "NEW" subroutine
            z80_cycle();
            vTaskDelay(60);
            addrMatch = zdbState.breakPoint;
            zxKeyboard = true;
            z80cpu_run();
            vTaskDelay(60);
            while (!keyboard_break() && addrMatch)
               taskYIELD();
            z80cpu_stop();
            vTaskDelay(60);
            vTaskResume(xuTermTask);
            zxKeyboard = false;
            regs_update(true);
            keyboard_flush(); // clear keyboard queue
            forceRedraw = REDRAW_CODE;
            break;
         case 'v': /// view zx80 screen
            while (keyboard_pressed())
               taskYIELD();
            vTaskResume(xLcdZxTask);
            zx50HzSignal = true;
            vTaskDelay(50);
            vTaskSuspend(xLcdZxTask);
            while (!keyboard_getch(&c))
               taskYIELD();
            regs_update(true);
            forceRedraw = REDRAW_UPDATE;
            break;
         default:
            break;
         }
         code_update(forceRedraw);
         regs_update(false);
         _z80_dump(zdbState.dumpAddress);
      }
      taskYIELD();
   }
}
void z80dbg(uint16_t addr)
{
   uint8_t cursorSize = uTerm.cursorSize;
   tprintf("\e[a"); // store system colours
   ucosRStream = stdio;
   stdio = &zgdbStream;
   uTerm.cursorSize = 0;
   zdb_init();
   vTaskDelay(10);
   /// draw layout
   code_update(REDRAW_CODE);
   regs_update(true);
   _z80_dump(zdbState.dumpAddress);
   zdb_process(addr);
   while (!keyboard_pressed())
      taskYIELD();
   uTerm.cursorSize = cursorSize;
   stdio = ucosRStream;
   tprintf("\e[r"); // restore system colours
   tprintf("\n");
   return;
}