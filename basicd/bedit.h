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
#ifndef _BEDIT_H_INCLUDED
#define _BEDIT_H_INCLUDED

#include "bcore.h"
#include "rshell.h"

enum _bas_msg_e
{
  BASIC_MSG_NORMAL,  
  BASIC_MSG_WARNING,  
  BASIC_MSG_ERROR,
};

char *get_basic_line(char *str,bool syntaxHl);
void basic_message(enum _bas_msg_e type,const char *str, ... );
cmd_err_t basic_exe(_cl_param_t *sParam);

extern _stream_io_t basicStream;
#endif //_BEDIT_H_INCLUDED