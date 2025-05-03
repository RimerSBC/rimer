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
/**
 * @file main.c
 * @author Sergey Sanders
 * @date 24 March 2024
 * @brief Rimer App 
 * ATSAME(D)5 chip
 * @see ATSAMD51 Datasheet
 */
#include "freeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "rshell.h"
#include "ucosR.h"

int main(void)
{
    AppGreeting = "-- Rimer App V0.63";
    bsp_init();
    xTaskCreate(ucosR_task, "ucosR", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    vTaskStartScheduler();
    while (1);
}

void vApplicationMallocFailedHook(void)
{
    asm("BKPT #0");
    while (1)
        asm("nop");
}
