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
#ifndef COLOURS_H_INCLUDED
#define COLOURS_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif
/**
enum _ansi_colours_e
{
  Black,
  Maroon,
  Green,
  Olive,
  Navy,
  Purple,
  Teal,
  Silver,
  Grey,
  Red,
  Lime,
  Yellow,
  Blue,
  Fuchsia,
  Aqua,
  White,
  Grey0,
  NavyBlue,
  DarkBlue,
  Blue3,
  Blue2,
  Blue1,
  DarkGreen,
  DeepSkyBlue42,
  DeepSkyBlue41,
  DeepSkyBlue4,
  DodgerBlue3,
  DodgerBlue2,
  Green4,
  SpringGreen4,
  Turquoise4,
  DeepSkyBlue31,
  DeepSkyBlue3,
  DodgerBlue1,
  Green31,
  SpringGreen31,
  DarkCyan,
  LightSeaGreen,
  DeepSkyBlue2,
  DeepSkyBlue1,
  Green3,
  SpringGreen3,
  SpringGreen21,
  Cyan3,
  DarkTurquoise,
  Turquoise2,
  Green1,
  SpringGreen2,
  SpringGreen1,
  MediumSpringGreen,
  Cyan2,
  Cyan1,
  DarkRed1,
  DeepPink41,
  Purple41,
  Purple4,
  Purple3,
  BlueViolet,
  Orange41,
  Grey37,
  MediumPurple4,
  SlateBlue31,
  SlateBlue3,
  RoyalBlue1,
  Chartreuse4,
  DarkSeaGreen41,
  PaleTurquoise4,
  SteelBlue,
  SteelBlue3,
  CornflowerBlue,
  Chartreuse31,
  DarkSeaGreen4,
  CadetBlue1,
  CadetBlue,
  SkyBlue3,
  SteelBlue11,
  Chartreuse3,
  PaleGreen31,
  SeaGreen3,
  Aquamarine3,
  MediumTurquoise,
  SteelBlue1,
  Chartreuse21,
  SeaGreen2,
  SeaGreen11,
  SeaGreen1,
  Aquamarine11,
  DarkSlateGray2,
  DarkRed,
  DeepPink42,
  DarkMagenta1,
  DarkMagenta,
  DarkViolet1,
  Purple1,
  Orange4,
  LightPink4,
  Plum4,
  MediumPurple31,
  MediumPurple3,
  SlateBlue1,
  Yellow41,
  Wheat4,
  Grey53,
  LightSlateGrey,
  MediumPurple,
  LightSlateBlue,
  Yellow4,
  DarkOliveGreen31,
  DarkSeaGreen,
  LightSkyBlue31,
  LightSkyBlue3,
  SkyBlue2,
  Chartreuse2,
  DarkOliveGreen32,
  PaleGreen3,
  DarkSeaGreen32,
  DarkSlateGray3,
  SkyBlue1,
  Chartreuse1,
  LightGreen,
  LightGreen1,
  PaleGreen11,
  Aquamarine1,
  DarkSlateGray1,
  Red31,
  DeepPink4,
  MediumVioletRed,
  Magenta31,
  DarkViolet,
  Purple2,
  DarkOrange31,
  IndianRed,
  HotPink31,
  MediumOrchid3,
  MediumOrchid,
  MediumPurple21,
  DarkGoldenrod,
  LightSalmon31,
  RosyBrown,
  Grey63,
  MediumPurple2,
  MediumPurple1,
  Gold31,
  DarkKhaki,
  NavajoWhite3,
  Grey69,
  LightSteelBlue3,
  LightSteelBlue,
  Yellow31,
  DarkOliveGreen3,
  DarkSeaGreen3,
  DarkSeaGreen21,
  LightCyan3,
  LightSkyBlue1,
  GreenYellow,
  DarkOliveGreen2,
  PaleGreen1,
  DarkSeaGreen2,
  DarkSeaGreen11,
  PaleTurquoise1,
  Red3,
  DeepPink31,
  DeepPink3,
  Magenta32,
  Magenta3,
  Magenta21,
  DarkOrange3,
  IndianRed2,
  HotPink3,
  HotPink2,
  Orchid,
  MediumOrchid11,
  Orange3,
  LightSalmon3,
  LightPink3,
  Pink3,
  Plum3,
  Violet,
  Gold3,
  LightGoldenrod3,
  Tan,
  MistyRose3,
  Thistle3,
  Plum2,
  Yellow3,
  Khaki3,
  LightGoldenrod21,
  LightYellow3,
  Grey84,
  LightSteelBlue1,
  Yellow2,
  DarkOliveGreen11,
  DarkOliveGreen1,
  DarkSeaGreen1,
  Honeydew2,
  LightCyan1,
  Red1,
  DeepPink2,
  DeepPink11,
  DeepPink1,
  Magenta2,
  Magenta1,
  OrangeRed1,
  IndianRed11,
  IndianRed1,
  HotPink1,
  HotPink,
  MediumOrchid1,
  DarkOrange,
  Salmon1,
  LightCoral,
  PaleVioletRed1,
  Orchid2,
  Orchid1,
  Orange1,
  SandyBrown,
  LightSalmon1,
  LightPink1,
  Pink1,
  Plum1,
  Gold1,
  LightGoldenrod22,
  LightGoldenrod2,
  NavajoWhite1,
  MistyRose1,
  Thistle1,
  Yellow1,
  LightGoldenrod1,
  Khaki1,
  Wheat1,
  Cornsilk1,
  Grey100,
  Grey3,
  Grey7,
  Grey11,
  Grey15,
  Grey19,
  Grey23,
  Grey27,
  Grey30,
  Grey35,
  Grey39,
  Grey42,
  Grey46,
  Grey50,
  Grey54,
  Grey58,
  Grey62,
  Grey66,
  Grey70,
  Grey74,
  Grey78,
  Grey82,
  Grey85,
  Grey89,
  Grey93
};// _ansi_colours_e;
*/
enum _sys_colours_e
{
    SC_BLACK,
    SC_BLUE,
    SC_RED,
    SC_MAGENTA,
    SC_GREEN,
    SC_CYAN,
    SC_YELLOW,
    SC_WHITE,
    SC_GREY,
    SC_BRIGHT_BLUE,
    SC_BRIGHT_RED,
    SC_BRIGHT_MAGENTA,
    SC_BRIGHT_GREEN,
    SC_BRIGHT_CYAN,
    SC_BRIGHT_YELLOW,
    SC_BRIGHT_WHITE,
}; //_sys_colours_e
#include "stdint.h" 
extern const uint8_t ANSI_pal256[256];

#ifdef __cplusplus
}
#endif
  
#endif /* COLOURS_H_INCLUDED */