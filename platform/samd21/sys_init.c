/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include "samd21.h"
#include "nvm_data.h"
#include "hal_config.h"


#ifdef HAL_CONFIG_CUSTOM_LED

// based upon 3MHz for TCC
#define LED_PWM_PERIOD    3000
// percentage of LED brigtness, 100 = Full brightness
#define LED_BRIGHTNESS	  12.5

void custom_hal_gpio_dap_status_toggle();
void custom_hal_gpio_dap_status_set();
#ifdef HAL_CONFIG_ENABLE_VCP
void custom_hal_gpio_vcp_status_toggle();
void custom_hal_gpio_vcp_status_write(int val);
#endif
static void led_custom_init();
#endif

//-----------------------------------------------------------------------------

#ifdef HAL_CONFIG_CUSTOM_LED
static void led_custom_init()
{
#ifdef HAL_BOARD_JEFF_PROBE
  /* Enable the APB clock for TCC0 */
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
  /* Enable GCLK1 and wire it up to TCC0 and TCC1. */
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |GCLK_CLKCTRL_ID_TCC0_TCC1 |
                    GCLK_CLKCTRL_GEN(0);
  /* Wait until the clock bus is synchronized. */
  while (GCLK->STATUS.bit.SYNCBUSY) {};

  /* Configure the clock prescaler for each TCC.
     This lets you divide up the clocks frequency to make the TCC count slower
     than the clock. In this case, I'm dividing the 48MHz clock by 16 making the
     TCC operate at 3MHz. This means each count (or "tick") is 0.33us.
  */
  TCC0->CTRLA.reg |= TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV16_Val);

  TCC0->PER.reg = LED_PWM_PERIOD;
  while (TCC0->SYNCBUSY.bit.PER) {};

  /* Use "Normal PWM" */
  TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  /* Wait for bus synchronization */
  while (TCC0->SYNCBUSY.bit.WAVE) {};

  /* n for CC[n] is determined by n = x % 4 where x is from WO[x]
   WO[x] comes from the peripheral multiplexer - we'll get to that in a second.
  */
  // CC3 used by DAP
  TCC0->CC[3].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100;
  while (TCC0->SYNCBUSY.bit.CC3) {};

  // CC2 used by VCP
#if defined(HAL_CONFIG_ENABLE_VCP)
  TCC0->CC[2].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100 ;
  while (TCC0->SYNCBUSY.bit.CC2) {};
#endif

  // CC0 used by Power LED
#if defined(HAL_CONFIG_HAS_PWR_STATUS)
  TCC0->CC[0].reg = LED_PWM_PERIOD * LED_BRIGHTNESS /100 ;
  while (TCC0->SYNCBUSY.bit.CC0) {};
#endif

  TCC0->CTRLA.reg |= (TCC_CTRLA_ENABLE);
  while (TCC0->SYNCBUSY.bit.ENABLE) {};

  HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  HAL_GPIO_DAP_STATUS_clr();

#if defined(HAL_CONFIG_ENABLE_VCP)
  /* set alt func */
  HAL_GPIO_VCP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  HAL_GPIO_VCP_STATUS_clr();
#endif

#if defined(HAL_CONFIG_HAS_PWR_STATUS)
  /* set alt func */
  HAL_GPIO_PWR_STATUS_pmuxen( HAL_GPIO_PMUX_F );
#endif

#endif
}

void custom_hal_gpio_dap_status_toggle()
{
  static int last_value = 0;
  last_value = ! last_value;
  if ( last_value) {
    HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  }else{
      HAL_GPIO_DAP_STATUS_clr();
    HAL_GPIO_DAP_STATUS_pmuxdis();
  }
}
void custom_hal_gpio_dap_status_set()
{
  HAL_GPIO_DAP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
}

#if defined(HAL_CONFIG_ENABLE_VCP)
void custom_hal_gpio_vcp_status_toggle()
{
  static int last_value = 0;
  last_value = ! last_value;
  custom_hal_gpio_vcp_status_write(last_value);
}

void custom_hal_gpio_vcp_status_write(int val)
{
  if ( val ) {
    HAL_GPIO_VCP_STATUS_pmuxen( HAL_GPIO_PMUX_F );
  }else{
      HAL_GPIO_VCP_STATUS_clr();
    HAL_GPIO_VCP_STATUS_pmuxdis();
  }
}

#endif

#if defined(HAL_CONFIG_HAS_PWR_STATUS)
void custom_hal_gpio_pwr_status_set()
{
  HAL_GPIO_PWR_STATUS_pmuxen( HAL_GPIO_PMUX_F );
}
#endif

#endif

void sys_init(void)
{
  uint32_t coarse, fine;

#ifdef HAL_CONFIG_CUSTOM_LED
  led_custom_init();
#endif

#ifdef DAP_CONFIG_SUPPLY_PWR
  HAL_GPIO_SUPPLY_PWR_out();
  HAL_GPIO_SUPPLY_PWR_clr();
#endif

#ifdef HAL_CONFIG_HAS_PWR_STATUS
  HAL_GPIO_PWR_STATUS_out();
#ifdef HAL_CONFIG_CUSTOM_LED
  custom_hal_gpio_pwr_status_set();
#else
  HAL_GPIO_PWR_STATUS_set();
#endif
#endif

  SYSCTRL->OSC8M.bit.PRESC = 0;

  SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
      SYSCTRL_INTFLAG_DFLLRDY;

  NVMCTRL->CTRLB.bit.RWS = 1;

  coarse = NVM_READ_CAL(NVM_DFLL48M_COARSE_CAL);
  fine = NVM_READ_CAL(NVM_DFLL48M_FINE_CAL);

  SYSCTRL->DFLLCTRL.reg = 0; // See Errata 9905
  while (0 == (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));

  SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_MUL(48000);
  SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(fine);

  SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE | SYSCTRL_DFLLCTRL_USBCRM |
      SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_BPLCKC | SYSCTRL_DFLLCTRL_CCDIS |
      SYSCTRL_DFLLCTRL_STABLE;

  while (0 == (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY));

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC(GCLK_SOURCE_DFLL48M) |
      GCLK_GENCTRL_RUNSTDBY | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
}

