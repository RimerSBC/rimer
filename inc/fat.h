/**
 * Copyright (c) 2024 Sergey Sanders
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _CMD_FAT_CTRL_INCLUDED
#define _CMD_FAT_CTRL_INCLUDED

#define SD_MMC_BLOCK_SIZE 512

#define SD_PIN_MCDA0    PORT_PA09
#define SD_PIN_MCDA1    PORT_PA10
#define SD_PIN_MCDA2    PORT_PA11
#define SD_PIN_MCDA3    PORT_PB10
#define SD_PIN_MCCK     PORT_PB11
#define SD_PIN_MCMD     PORT_PA08
#define SD_PIN_CD       PORT_PA06
#define SD_PORT_CTRLA   PORT->Group[0] // porta
#define SD_PORT_CTRLB   PORT->Group[1] // portb

#define SYS_SD_SLOT    0

void fat_task(void* vParam );
bool sd_info_print(uint8_t slot);
extern uint8_t sd_mmc_block[SD_MMC_BLOCK_SIZE];

#endif //_CMD_FAT_CTRL_INCLUDED