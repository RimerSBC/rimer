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
#include "bprog_rom.h"

const _bas_rom_t ROM_ctree = {
    .name = "ctree",
    .prog = "\
10 input \"How big is your tree (3-40)?\",size\n\
20 if (size<3) or (size>40) then print \"oi oi oi!\":stop\n\
25 size = size-1\n\
110 ? ink(5);\"Merry christmass !!!\":? :?\n\
120 for i = 0 to size\n\
130 for j = 0 to size-i:print \" \";:next j ' space\n\
135 if i = 0 then print ink(1);\"@\": next i\n\
140 for j = 0 to i*2\n\
143 print ink(rnd(3)+1);\"*\";\n\
145 next j ' tree\n\
148 'for j = 0 to size:print \" \";:next j ' space\n\
149 print 'next line\n\
150 next i\n\
153 print ink(7);\n\
155 for j = 0 to 2\n\
160 for i=0 to size-1: ? \" \";:next i\n\
170 print \"|||\"\n\
180 next j\n\
990 stop\n\
"};

const _bas_rom_t ROM_snake = {
    .name = "snake",
    .prog = "\
10 XSIZE_=40 : YSIZE_=20 : MAX_LENGTH = 254: MAX_LEVEL = 9\n\
15 'def SNAKEDELAY#(lvl) = 10-lvl\n\
20 print \" Snake game V1.0 \",\" Use WASD keys for snake control\"\n\
30 input \"Enter starting level (1-5)\",lvl\n\
40 if lvl > 5 then print \"Don\'t flatter yourself...\":sleep(2000):lvl = 5\n\
50 if lvl < 1 then print \"Chicken...\":sleep(2000):lvl = 1\n\
60 LEVEL = lvl\n\
70 delay# = 30-lvl*3\n\
80 dim snake_[256,2] = XSIZE_/2,YSIZE_/2+2\n\
90 head_ = 0 : length_ = 1 : dir_ = 0 rem 0 - up, 1 - down, 2 - right, 3 - left\n\
100 gosub 950\n\
110 gosub 840\n\
130 print at(snake_[head_,0],snake_[head_,1]);ink(5);\"O\";\n\
135 sleep(10-LEVEL):\n\
140 gosub 500\n\
145 print at(snake_[head_,0],snake_[head_,1]);ink(4);\"*\";\n\
150 head_ = head_ + 1\n\
155 key_ = inkey()\n\
161 if key_ = 119 or key_ = 65 then dir_ = 0: goto 170\n\
162 if key_ = 115 or key_ = 66 then dir_ = 1: goto 170\n\
163 if key_ = 100 or key_ = 67 then dir_ = 2: goto 170\n\
164 if key_ = 97 or key_ = 68 then  dir_ = 3: goto 170\n\
165 if key_ = 32 then goto 1000\n\
170 snake_[head_,0] = snake_[head_-1,0]: snake_[head_,1] = snake_[head_-1,1]\n\
200 if dir_ = 0 then snake_[head_,1] = snake_[head_,1] - 1: goto 240\n\
210 if dir_ = 1 then snake_[head_,1] = snake_[head_,1] + 1: goto 240\n\
220 if dir_ = 2 then snake_[head_,0] = snake_[head_,0] + 1: goto 240\n\
230 if dir_ = 3 then snake_[head_,0] = snake_[head_,0] - 1: goto 240\n\
240 if snake_[head_,0] = rabbitX_ and snake_[head_,1] = rabbitY_ then gosub 800:goto 130\n\
270 print at(snake_[head_-length_,0],snake_[head_-length_,1]);\" \";'erase the tail\n\
280 snake_[head_-length_,0] = 0 : snake_[head_-length_,1] = 0\n\
290 goto 130\n\
500 if (snake_[head_,0] = 1) or (snake_[head_,0] = XSIZE_) or (snake_[head_,1] = 1) or (snake_[head_,1] = YSIZE_) then goto 999\n\
510 for c=0 to 255:if c <> head_ and snake_[head_,0] = snake_[c,0] and snake_[head_,1] = snake_[c,1] then goto 999\n\
520 next c\n\
530 return\n\
666 goto 1000\n\
800 gosub 900\n\
810 length_ = length_ + 1:print ink(7);at(XSIZE_/2-5,YSIZE_);\" LENGTH : \";length_;\" \";\n\
820 if length_ < MAX_LENGTH then return\n\
830 LEVEL = LEVEL + 1: if LEVEL > MAX_LEVEL then print at(XSIZE_/2-5,YSIZE_/2);ink(3);\"You WIN!\": goto 1100\n\
835 gosub 950\n\
840 for r=0 to 255: snake_[r,0] = 0: snake_[r,1] = 0:next r\n\
850 snake_[0,0] = XSIZE_/2,YSIZE_/2+2\n\
860 head_ = 0:length_ = 1\n\
870 return\n\
900 rabbitX_ = rnd(XSIZE_-2)+2 : rabbitY_ = rnd(YSIZE_-2)+2\n\
910 for r=0 to 255:if rabbitX_ = snake_[r,0] and rabbitY_ = snake_[r,1] then goto 900\n\
911 next r\n\
920 print at(rabbitX_,rabbitY_);ink(6);\"@\";:return\n\
950 cls:print ink(7);\n\
970 for i=2 to XSIZE_-1: print at(i,1);\"-\";at(i,YSIZE_);\"-\";: next i\n\
975 print at(XSIZE_/2-4,1);\" LEVEL : \";int(LEVEL);\" \";\n\
976 'sleep(50)\n\
980 for i=2 to YSIZE_-1: print at(1,i);\"|\";at(XSIZE_,i);\"|\";: next i\n\
990 'print at(1,1);\"+\";at(1,YSIZE_);\"+\";at(XSIZE_,1);\"+\";at(XSIZE_,YSIZE_);\"+\";\n\
991 gosub 900\n\
995 return\n\
999 print at(snake_[head_,0],snake_[head_,1]);ink(2);\"X\"\n\
1000 print at(XSIZE_/2-5,YSIZE_/2);ink(2);\"Game OVER!\";\n\
1100 print ink(7);at(XSIZE_/2-7,YSIZE_/2+1);\"Press any key\";\n\
1110 if !inkey() then goto 1110 \n\
1120 at(1,YSIZE_):stop\n\
"};
const _bas_rom_t ROM_bounce = {
    .name = "bounce",
    .prog = "\
10 cls\n\
20 'for i=1 to 7:print ink(i);int(i);:next i\n\
30 'key_ = inkey()\n\
40 'if key_ then ? at(10,10);key_;\"  \"\n\
50 'if key_ <> 48 then goto 30\n\
60 xMax_=38:yMax_=18\n\
70 x_=rnd(xMax_)+1:y_ = rnd(yMax_)+1\n\
80 dirX_=rnd(2):dirY_ = rnd(2)\n\
90 if x_=xMax_+int(rnd(4)-2) or x_>xMax_+1 then dirX_=0:goto 110\n\
100 if x_=1 then dirX_ = 1\n\
110 if y_=yMax_+int(rnd(4)-2) or y_>yMax_+1 then dirY_=0:goto 130\n\
120 if y_=1 then dirY_=1\n\
130 if dirX_ then x_=x_+1: goto 200\n\
140 x_=x_-1\n\
200 if dirY_ then y_=y_+1: goto 220\n\
210 y_=y_-1\n\
220 print at(x_,y_);\"O\";'Put the character\n\
230 sleep(10)\n\
240 print ink(rnd(7)+1);at(x_,y_);\".\";' erase the character\n\
250 if inkey <> 48 then goto 90' press 0 to stop\n\
"};
/*
1 cls
2 for i=1 to 7:print ink(i);int(i);:next i
3 key_ = inkey()
4 if key_ then ? at(10,10);key_;"  "
5 if key_ <> 48 then goto 3
10 xMax_ = 45:yMax_ = 20
20 x_ = rnd(xMax_)+1:y_ = rnd(yMax_)+1
#22 ? x_;" ";y_
30 dirX_ = rnd(2):dirY_ = rnd(2)
40 if x_ = xMax_ + int(rnd(6)-3) or x_>xMax_+6 then dirX_ = 0:goto 60
50 if x_ = 1 then dirX_ = 1
60 if y_ = yMax_ then dirY_ = 0: goto 80
70 if y_ = 1 then dirY_ = 1 
80 if dirX_ then x_ = x_+1: goto 100
90 x_ = x_-1
100 if dirY_ then y_ = y_+1: goto 120
110 y_ = y_-1
120 print at(x_,y_);"O"'Put the character
130 sleep(50)
140 print ink(rnd(8));at(x_,y_);"*"' erase the character
150 if inkey <> 27 then goto 40
*/
/*

1000 REM Mandelbrot Set Project
1010 REM Quite BASIC Math Project
1020 REM ------------------------ 
1030 CLS
1040 PRINT "This program plots a graphical representation of the famous Mandelbrot set.  It takes a while to finish so have patience and don't have too high expectations;  the graphics resolution is not very high on our canvas."
2000 REM Initialize the color palette
2010 GOSUB 3000
2020 REM L is the maximum iterations to try
2030 LET L = 100
2040 FOR I = 0 TO 100
2050 FOR J = 0 TO 100
2060 REM Map from pixel coordinates (I,J) to math (U,V)
2060 LET U = I / 50 - 1.5
2070 LET V = J / 50 - 1
2080 LET X = U
2090 LET Y = V
2100 LET N = 0
2110 REM Inner iteration loop starts here 
2120 LET R = X * X
2130 LET Q = Y * Y
2140 IF R + Q > 4 OR N >= L THEN GOTO 2190
2150 LET Y = 2 * X * Y + V
2160 LET X = R - Q + U
2170 LET N = N + 1
2180 GOTO 2120
2190 REM Compute the color to plot
2200 IF N < 10 THEN LET C = "black" ELSE LET C = P[ROUND(8 * (N-10) / (L-10))]
2210 PLOT I, J, C 
2220 NEXT J
2230 NEXT I
2240 END
3000 REM Subroutine -- Set up Palette
3010 ARRAY P
3020 LET P[0] = "black"
3030 LET P[1] = "magenta"
3040 LET P[2] = "blue"
3050 LET P[3] = "green"
3060 LET P[4] = "cyan"
3070 LET P[5] = "red"
3080 LET P[6] = "orange"
3090 LET P[7] = "yellow"
3090 LET P[8] = "white"
3100 RETURN
 
10 cls
20 ' L is the maximum iterations to try
30 L = 100
40 for I = 0 TO 200
50 for J = 0 TO 200
60 ' Map from pixel coordinates (I,J) to math (U,V)
60 U = I / 50 - 1.5
70 V = J / 50 - 1
80 X = U
90 Y = V
100 N = 0
110 ' Inner iteration loop starts here 
120 R = X * X
130 Q = Y * Y
140 if R + Q > 4 or N >= L then goto 190
150 Y = 2 * X * Y + V
160 X = R - Q + U
170 N = N + 1
180 goto 120
190 ' Compute the color to plot
200 if N < 10 then ink(0):goto 210
205 ink(7 * (N-10) / (L-10))
210 plot(I, J)
220 next J
230 next I
240 stop

10 XSIZE_=40 : YSIZE_=20 : MAX_LENGTH = 254: MAX_LEVEL = 9
15 'def SNAKEDELAY#(lvl) = 10-lvl
20 print " Snake game V1.0 "," Use WASD keys for snake control"
30 input "Enter starting level (1-5)",lvl
40 if lvl > 5 then print "Don't flatter yourself...":sleep(2000):lvl = 5
50 if lvl < 1 then print "Chicken...":sleep(2000):lvl = 1
60 LEVEL = lvl
70 delay# = 30-lvl*3
80 dim snake_[256,2] = XSIZE_/2,YSIZE_/2+2
90 head_ = 0 : length_ = 1 : dir_ = 0 rem 0 - up, 1 - down, 2 - right, 3 - left
100 gosub 950
110 gosub 840
130 print at(snake_[head_,0],snake_[head_,1]);ink(5);"O";
135 sleep(10-LEVEL):
140 gosub 500
145 print at(snake_[head_,0],snake_[head_,1]);ink(4);"*";
150 head_ = head_ + 1
155 key_ = inkey()
161 if key_ = 119 or key_ = 65 then dir_ = 0: goto 170
162 if key_ = 115 or key_ = 66 then dir_ = 1: goto 170
163 if key_ = 100 or key_ = 67 then dir_ = 2: goto 170
164 if key_ = 97 or key_ = 68 then  dir_ = 3: goto 170
165 if key_ = 32 then goto 1000
170 snake_[head_,0] = snake_[head_-1,0]: snake_[head_,1] = snake_[head_-1,1]
200 if dir_ = 0 then snake_[head_,1] = snake_[head_,1] - 1: goto 240
210 if dir_ = 1 then snake_[head_,1] = snake_[head_,1] + 1: goto 240
220 if dir_ = 2 then snake_[head_,0] = snake_[head_,0] + 1: goto 240
230 if dir_ = 3 then snake_[head_,0] = snake_[head_,0] - 1: goto 240
240 if snake_[head_,0] = rabbitX_ and snake_[head_,1] = rabbitY_ then gosub 800:goto 130
270 print at(snake_[head_-length_,0],snake_[head_-length_,1]);" ";'erase the tail
280 snake_[head_-length_,0] = 0 : snake_[head_-length_,1] = 0
290 goto 130
500 if (snake_[head_,0] = 1) or (snake_[head_,0] = XSIZE_) or (snake_[head_,1] = 1) or (snake_[head_,1] = YSIZE_) then goto 999
510 for c=0 to 255:if c <> head_ and snake_[head_,0] = snake_[c,0] and snake_[head_,1] = snake_[c,1] then goto 999
520 next c
530 return
666 goto 1000
800 gosub 900
810 length_ = length_ + 1:print ink(7);at(XSIZE_/2-5,YSIZE_);" LENGTH : ";length_;" ";
820 if length_ < MAX_LENGTH then return
830 LEVEL = LEVEL + 1: if LEVEL > MAX_LEVEL then print at(XSIZE_/2-5,YSIZE_/2);ink(3);"You WIN!": goto 1100
835 gosub 950
840 for r=0 to 255: snake_[r,0] = 0: snake_[r,1] = 0:next r
850 snake_[0,0] = XSIZE_/2,YSIZE_/2+2
860 head_ = 0:length_ = 1
870 return
900 rabbitX_ = rnd(XSIZE_-2)+2 : rabbitY_ = rnd(YSIZE_-2)+2
910 for r=0 to 255:if rabbitX_ = snake_[r,0] and rabbitY_ = snake_[r,1] then goto 900
911 next r
920 print at(rabbitX_,rabbitY_);ink(6);"@";:return
950 cls:print ink(7);
970 for i=2 to XSIZE_-1: print at(i,1);"-";at(i,YSIZE_);"-";: next i
975 print at(XSIZE_/2-4,1);" LEVEL : ";int(LEVEL);" ";
976 'sleep(50)
980 for i=2 to YSIZE_-1: print at(1,i);"|";at(XSIZE_,i);"|";: next i
990 'print at(1,1);"+";at(1,YSIZE_);"+";at(XSIZE_,1);"+";at(XSIZE_,YSIZE_);"+"; 
991 gosub 900
995 return
999 print at(snake_[head_,0],snake_[head_,1]);ink(2);"X"
1000 print at(XSIZE_/2-5,YSIZE_/2);ink(2);"Game OVER!";
1100 print ink(7);at(XSIZE_/2-7,YSIZE_/2+1);"Press any key";
1110 if !inkey() then goto 1110 
1120 at(1,YSIZE_):stop

*/