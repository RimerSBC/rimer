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

#include "berror.h"

_bas_err_e BasicError;

const char *BErrorText[BASIC_ERR_COUNT] =
{
    "",
    "File not found",
    "Cannot open file",
    "No line number",
    "Duplicate line number",
    "Invalid line number",
    "Invalid delimiter",    
    "String too long",
    "Out of memory",
    "Type mismatch",
    "Wrong usage of string var",
    "Missing parenthese",
    "Too few arguments",
    "Too many arguments",
    "Missing operand",
    "Missing operator",
    "Missing \'=\'",
    "Unknown variable",
    "Unknown operator",
    "Unknown function",
    "Using reserved name",
    "Missing \"then\"",
    "RPN queue is empty",
    "RPN queue is full",
    "RPN stack is full",
    "Divide by zero",
    "Incomplete FOR loop",
    "GOSUB stack overflow",
    "RETURN without GOSUB",
    "Array redefine",
    "Array out of range",
    "Wrong array dimentions",
    "Variable redefine",
    "Variable out of range",
    "Function redefine",
    "Max aruments exceeded",
    "System call error",
    "Illigal \"run\" command",
};
