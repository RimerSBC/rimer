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

#ifndef _BERROR_H_INCLUDED
#define _BERROR_H_INCLUDED

typedef enum
{
    BASIC_ERR_NONE,
    BASIC_ERR_FILE_NOT_FOUND,
    BASIC_ERR_FILE_CANT_OPEN,
    BASIC_ERR_LOAD_NONUMBER,
    BASIC_ERR_LOAD_DUPLICATE,
    BASIC_ERR_INVALID_LINE,
    BASIC_ERR_INVALID_DELIMITER,    
    BASIC_ERR_STRING_LENGTH,
    BASIC_ERR_MEM_OUT,
    BASIC_ERR_TYPE_MISMATCH,
    BASIC_ERR_STRING_MISMATCH,
    BASIC_ERR_PAR_MISMATCH,
    BASIC_ERR_FEW_ARGUMENTS,
    BASIC_ERR_MANY_ARGUMENTS,
    BASIC_ERR_MISSING_OPERAND,
    BASIC_ERR_MISSING_OPERATOR,
    BASIC_ERR_MISSING_EQUAL,
    BASIC_ERR_UNKNOWN_VAR,
    BASIC_ERR_UNKNOWN_OP,
    BASIC_ERR_UNKNOWN_FUNC,
    BASIC_ERR_RESERVED_NAME,
    BASIC_ERR_NO_THEN,
    BASIC_ERR_QUEUE_EMPTY,
    BASIC_ERR_QUEUE_FULL,
    BASIC_ERR_STACK_FULL,
    BASIC_ERR_DIV_ZERO,
    BASIC_ERR_INCOMPLETE_FOR,
    BASIC_ERR_GOSUB_OVERFLOW,
    BASIC_ERR_RETURN_NO_GOSUB,
    BASIC_ERR_ARRAY_REDEFINE,
    BASIC_ERR_ARRAY_OUTOFRANGE,
    BASIC_ERR_ARRAY_DIMENTION,
    BASIC_ERR_VAR_REDEFINE,
    BASIC_ERR_VAR_OUTOFRANGE,
    BASIC_ERR_DEFFN_REDEFINE,
    BASIC_ERR_DEFFN_ARGUMENTS,
    BASIC_ERR_SYSCALL_ERROR,
    BASIC_ERR_RUN_ERROR,
    BASIC_ERR_COUNT
}_bas_err_e;

extern _bas_err_e BasicError;
extern const char *BErrorText[BASIC_ERR_COUNT];

#endif // _BERROR_H_INCLUDED