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
 * @file rshell.c
 * @author Sergey Sanders
 * @date April 2023
 * @brief * rshell interface "Analog IO"
 *   analog input/output interface
 *
 */
#include "FreeRTOS.h"
#include "bsp.h"
#include "keyboard.h"
#include "rshell.h"
#include "task.h"
#include "tstring.h"
#include "uterm.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "component/nvmctrl.h"

static bool iface_aio_init(bool verbose);
static cmd_err_t cmd_aio_adc(_cl_param_t *sParam);
static cmd_err_t cmd_aio_dac(_cl_param_t *sParam);
static cmd_err_t cmd_aio_vbat(_cl_param_t *sParam);
static cmd_err_t cmd_aio_vio(_cl_param_t *sParam);
static cmd_err_t cmd_aio_beep(_cl_param_t *sParam);
_iface_t ifaceAIO =
    {
        .name = "aio",
        .prompt = NULL,
        .desc = "Analog IO ports control",
        .init = iface_aio_init,
        .cmdList =
            {
                {.name = "adc", .desc = "ADC voltage", .func = cmd_aio_adc},
                {.name = "dac", .desc = "DAC voltage", .func = cmd_aio_dac},
                {.name = "vbat", .desc = "Battery voltage", .func = cmd_aio_vbat},
                {.name = "vio", .desc = "Vio voltage", .func = cmd_aio_vio},
                {.name = "beep", .desc = "Beep", .func = cmd_aio_beep},
                {.name = NULL, .func = NULL},
            }};
#define AIO_VREF_VOLTAGE 3.0
#define AIO_ADC_LSB (AIO_VREF_VOLTAGE / 4096.0)
#define AIO_DAC_LSB (AIO_VREF_VOLTAGE / 4096.0)

static bool iface_aio_init(bool verbose)
{
   static bool initialized = false;
   if (initialized)
      return true;

   AIO_CONFIG();
   // ADC0 config
   ADC0->REFCTRL.reg = ADC_REFCTRL_REFSEL_AREFA;
   ADC0->CALIB.reg = ADC_CALIB_BIASCOMP(calData->ADC0_BIASCOMP) | ADC_CALIB_BIASR2R(calData->ADC0_BIASR2R) | ADC_CALIB_BIASREFBUF(calData->ADC0_BIASREFBUF);
   ADC0->CTRLB.reg = ADC_CTRLB_RESSEL_16BIT;                             // 16 bit average
   ADC0->AVGCTRL.reg = ADC_AVGCTRL_ADJRES(3) | ADC_AVGCTRL_SAMPLENUM(3); // 8 samples
   ADC0->INPUTCTRL.bit.MUXNEG = ADC_INPUTCTRL_MUXNEG_GND_Val;
   ADC0->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_AIN4_Val;
   ADC0->CTRLA.bit.ENABLE = 1;
   while (ADC0->SYNCBUSY.bit.ENABLE)
      asm("nop");
   ADC0->SWTRIG.bit.START = 1;
   // ADC1 config
   ADC1->REFCTRL.reg = ADC_REFCTRL_REFSEL_AREFA;
   ADC1->CALIB.reg = ADC_CALIB_BIASCOMP(calData->ADC1_BIASCOMP) | ADC_CALIB_BIASR2R(calData->ADC1_BIASR2R) | ADC_CALIB_BIASREFBUF(calData->ADC1_BIASREFBUF);
   ADC1->CTRLB.reg = ADC_CTRLB_RESSEL_16BIT;                             // 16 bit average
   ADC1->AVGCTRL.reg = ADC_AVGCTRL_ADJRES(3) | ADC_AVGCTRL_SAMPLENUM(3); // 8 samples
   ADC1->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_AIN6;
   ADC1->CTRLA.bit.ENABLE = 1;
   while (ADC1->SYNCBUSY.bit.ENABLE)
      asm("nop");
   ADC1->SWTRIG.bit.START = 1;

   // DAC0 config
   DAC->CTRLB.reg = DAC_CTRLB_REFSEL(0); // VREF extern
   // DAC->DACCTRL[0].bit.FEXT = 1;
   DAC->DACCTRL[0].bit.ENABLE = 1;
   DAC->DACCTRL[0].bit.REFRESH = 1;
   DAC->DACCTRL[1].bit.ENABLE = 1;
   DAC->DACCTRL[1].bit.REFRESH = 1;
   DAC->CTRLA.bit.ENABLE = 1;
   while (DAC->SYNCBUSY.bit.ENABLE)
      asm("nop");

   DAC->DATA[1].reg = 0;
   vTaskDelay(1);
   DAC->DATA[1].reg = 2000;
   vTaskDelay(1);

   initialized = true;
   if (verbose)
      tprintf("Interface \"io\" initialized.\n");
   return true;
}

static cmd_err_t cmd_aio_adc(_cl_param_t *sParam)
{
   ADC0->INTFLAG.reg = ADC0->INTFLAG.reg;
   ADC0->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_AIN4_Val;
   while (ADC0->SYNCBUSY.bit.INPUTCTRL)
      asm("nop");
   ADC0->SWTRIG.bit.START = 1;
   while (!ADC0->INTFLAG.bit.RESRDY)
      taskYIELD();
   vTaskDelay(1);
   tprintf("%s\n", tftoa((ADC0->RESULT.reg * AIO_ADC_LSB), 3));
   return CMD_NO_ERR;
}

static cmd_err_t cmd_aio_dac(_cl_param_t *sParam)
{
   uint16_t regVal = 0;
   if (sParam->argc)
   {
      float voltage = atof(sParam->argv[0]);
      regVal = (uint16_t)(voltage / AIO_DAC_LSB);
      if (regVal > 4095)
         regVal = 4095;
      DAC->DATA[0].reg = regVal;
      while (!DAC->STATUS.bit.EOC0)
         asm("nop");
   }
   regVal = DAC->RESULT[0].reg;
   tprintf("%s\n", tftoa(regVal * AIO_DAC_LSB, 0));
   return CMD_NO_ERR;
}

static cmd_err_t cmd_aio_beep(_cl_param_t *sParam)
{
   uint16_t i;
   for (i = 0; i < 250; i++)
   {
      DAC->DATA[1].reg = sysConf.volume;
      vTaskDelay(1);
      DAC->DATA[1].reg = 0;
      vTaskDelay(1);
   }

   return CMD_NO_ERR;
}

static cmd_err_t cmd_aio_vbat(_cl_param_t *sParam)
{
   ADC1->INTFLAG.reg = ADC1->INTFLAG.reg;
   ADC1->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_AIN6_Val;
   while (ADC1->SYNCBUSY.bit.INPUTCTRL)
      asm("nop");
   ADC1->SWTRIG.bit.START = 1;
   while (!ADC1->INTFLAG.bit.RESRDY)
      taskYIELD();
   tprintf("%s\n", tftoa((ADC1->RESULT.reg * AIO_ADC_LSB) * 2, 3)); /// Vbat read from a voltage divider.
   return CMD_NO_ERR;
}

static cmd_err_t cmd_aio_vio(_cl_param_t *sParam)
{
   ADC1->INTFLAG.reg = ADC1->INTFLAG.reg;
   ADC1->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_AIN7_Val;
   while (ADC1->SYNCBUSY.bit.INPUTCTRL)
      asm("nop");
   ADC1->SWTRIG.bit.START = 1;
   while (!ADC1->INTFLAG.bit.RESRDY)
      taskYIELD();
   tprintf("%s\n", tftoa((ADC1->RESULT.reg * AIO_ADC_LSB) * 2, 3));
   return CMD_NO_ERR;
}
