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
#ifndef _BHIGHLIGHT_H_INCLUDED
#define _BHIGHLIGHT_H_INCLUDED
enum
{
  SYNCOL_BACKGROUND,
  SYNCOL_DEFAULT,
  SYNCOL_WARNING,
  SYNCOL_ERROR,
  SYNCOL_COMMENT,
  SYNCOL_DELIMITER,
  SYNCOL_BRACKET,
  SYNCOL_LINE_NUM,
  SYNCOL_NUMBER,
  SYNCOL_STRING,
  SYNCOL_OPERATOR,
  SYNCOL_CONTROL,
  SYNCOL_FUNC_PRIMARY,
  SYNCOL_FUNC_SECONDARY,
  SYNCOL_COUNT
};

extern const uint8_t SyntaxHLdark[SYNCOL_COUNT];
extern uint8_t *HLScheme;

void highlight(char *str);
#endif // _BHIGHLIGHT_H_INCLUDED
