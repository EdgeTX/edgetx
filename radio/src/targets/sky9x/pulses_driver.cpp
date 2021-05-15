/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "edgetx.h"

void setExternalModulePolarity()
{
  Pwm * pwmptr = PWM;
  pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = GET_MODULE_PPM_DELAY(EXTERNAL_MODULE) * 2; // Duty in half uS
  if (GET_MODULE_PPM_POLARITY(EXTERNAL_MODULE))
    pwmptr->PWM_CH_NUM[3].PWM_CMR &= ~0x00000200;   // CPOL
  else
    pwmptr->PWM_CH_NUM[3].PWM_CMR |= 0x00000200;    // CPOL
}

void setExtraModulePolarity()
{
  Pwm * pwmptr = PWM;
  pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = GET_MODULE_PPM_DELAY(EXTRA_MODULE) * 2; // Duty in half uS
  if (GET_MODULE_PPM_POLARITY(EXTRA_MODULE))
    pwmptr->PWM_CH_NUM[1].PWM_CMR &= ~0x00000200;   // CPOL
  else
    pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200;    // CPOL
}

void module_output_active()
{
  Pio *pioptr = PIOA;
  pioptr->PIO_ABCDSR[0] &= ~PIO_PA17;            // Peripheral C
  pioptr->PIO_ABCDSR[1] |= PIO_PA17;             // Peripheral C
  pioptr->PIO_PDR = PIO_PA17;                    // Disable bit A17 Assign to peripheral
#if defined(REVX)
  if (g_model.moduleData[EXTERNAL_MODULE].ppm.outputType) {
    pioptr->PIO_MDDR = PIO_PA17;                 // Push Pull O/p in A17
  }
  else {
    pioptr->PIO_MDER = PIO_PA17;                 // Open Drain O/p in A17
  }
#else
  pioptr->PIO_MDDR = PIO_PA17;                   // Push Pull O/p in A17
#endif
  pioptr->PIO_PUER = PIO_PA17;                   // With pull up
}

void init_main_ppm(uint32_t period, uint32_t out_enable)
{
  Pwm * pwmptr;

  if (out_enable) {
    module_output_active();
  }

  pwmptr = PWM;
  // PWM3 for PPM output
  pwmptr->PWM_CH_NUM[3].PWM_CMR = 0x0004000B;                          // CLKA
  pwmptr->PWM_CH_NUM[3].PWM_CPDR = period;                             // Period in half uS
  pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period;                          // Period in half uS
  pwmptr->PWM_CH_NUM[3].PWM_CDTY = GET_MODULE_PPM_DELAY(EXTERNAL_MODULE) * 2; // Duty in half uS
  pwmptr->PWM_ENA = PWM_ENA_CHID3;                                     // Enable channel 3
  pwmptr->PWM_IER1 = PWM_IER1_CHID3;

  setExternalModulePolarity();

  NVIC_SetPriority(PWM_IRQn, 3);
  NVIC_EnableIRQ(PWM_IRQn);
}

void disable_main_ppm()
{
  Pio * pioptr = PIOA;
  pioptr->PIO_PER = PIO_PA17;                                          // Assign A17 to PIO
  PWM->PWM_IDR1 = PWM_IDR1_CHID3;
}

void init_second_ppm(uint32_t period)
{
  // PWM1 for PPM2
  Pwm * pwmptr = PWM;
  configure_pins(PIO_PC15, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_B | PIN_PORTC | PIN_NO_PULLUP);
  pwmptr->PWM_CH_NUM[1].PWM_CMR = 0x0000000B;                          // CLKB
  if (!GET_MODULE_PPM_POLARITY(EXTRA_MODULE)) {
    pwmptr->PWM_CH_NUM[1].PWM_CMR |= 0x00000200;                       // CPOL
  }
  pwmptr->PWM_CH_NUM[1].PWM_CPDR = period;                             // Period
  pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = period;                          // Period
  pwmptr->PWM_CH_NUM[1].PWM_CDTY = GET_MODULE_PPM_DELAY(EXTRA_MODULE)*2;      // Duty
  pwmptr->PWM_CH_NUM[1].PWM_CDTYUPD = GET_MODULE_PPM_DELAY(EXTRA_MODULE)*2;   // Duty
  pwmptr->PWM_ENA = PWM_ENA_CHID1;                                     // Enable channel 1
  pwmptr->PWM_IER1 = PWM_IER1_CHID1;

  setExtraModulePolarity();
}

/*
// TODO re-add it later
void disable_second_ppm()
{
  Pio * pioptr = PIOC;
  pioptr->PIO_PER = PIO_PC15;           // Assign C17 to PIO
  PWM->PWM_IDR1 = PWM_IDR1_CHID1;
}

void init_module_timer(uint32_t port, uint32_t period, uint8_t state)
{
  if (port == EXTERNAL_MODULE) {
    // TODO use period here
    init_main_ppm(3000, 0);
  }
}
*/

void extmodulePpmStart()
{
  init_main_ppm(3000, 1);
}

void extramodulePpmStart()
{
  init_second_ppm(3000);
}

// Initialise the SSC to allow PXX output.
// TD is on PA17, peripheral A
void init_ssc(uint8_t baudrateDiv1000)
{
  Ssc *sscptr;

  PMC->PMC_PCER0 |= 0x00400000L;        // Enable peripheral clock to SSC

  configure_pins(PIO_PA17, PIN_PERIPHERAL | PIN_INPUT | PIN_PER_A | PIN_PORTA);

  sscptr = SSC;
  sscptr->SSC_THR = 0xFF;                    // Make the output high.
  sscptr->SSC_TFMR = 0x00000027;        //  0000 0000 0000 0000 0000 0000 1010 0111 (8 bit data, lsb)
  sscptr->SSC_CMR = Master_frequency / (1000 * baudrateDiv1000 * 2);
  sscptr->SSC_TCMR = 0;
  sscptr->SSC_CR = SSC_CR_TXEN;

#if defined(REVX)
  if (isModuleMultimodule(EXTERNAL_MODULE)) {
    PIOA->PIO_MDDR = PIO_PA17;                 // Push Pull O/p in A17
  } else {
    PIOA->PIO_MDER = PIO_PA17;						// Open Drain O/p in A17
  }
#else
  PIOA->PIO_MDDR = PIO_PA17;						// Push Pull O/p in A17
#endif
}

void disable_ssc()
{
  Pio *pioptr;
  Ssc *sscptr;

  // Revert back to pwm output
  pioptr = PIOA;
  pioptr->PIO_PER = PIO_PA17;           // Assign A17 to PIO

  sscptr = SSC;
  sscptr->SSC_CR = SSC_CR_TXDIS;
}

void extmodulePxx1PulsesStart()
{
  init_main_ppm(2500 * 2, 0);
  init_ssc(125); // 8us per bit
}

void extmoduleStop()
{
  disable_ssc();
  disable_main_ppm();
}

void extmoduleSerialStart(uint32_t baudrate, uint32_t period_half_us, bool inverted)
{
  if (baudrate == 125000) {
    // TODO init_main_ppm could take the period as parameter?
    init_main_ppm(2500 * 2, 0);
    init_ssc(125);
  }
  else {
    init_main_ppm(3500 * 2, 0);
    init_ssc(100);
  }
}

void extmoduleSendNextFrame()
{
}

#if !defined(SIMU)
extern "C" void PWM_IRQHandler(void)
{
  Pwm * pwmptr = PWM;
  uint32_t reason = pwmptr->PWM_ISR1;
  uint32_t period;

  if (reason & PWM_ISR1_CHID3) {
    // Use the current protocol, don't switch until set_up_pulses
    switch (moduleState[EXTERNAL_MODULE].protocol) {
      case PROTOCOL_CHANNELS_PXX1_PULSES:
        // Alternate periods of 6.5mS and 2.5 mS
        period = pwmptr->PWM_CH_NUM[3].PWM_CPDR;
        if (period == 2500 * 2) {
          period = 6500 * 2;
        }
        else {
          period = 2500 * 2;
        }
        pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period; // Period in half uS
        if (period != 2500 * 2) {
          setupPulsesExternalModule();
          setExternalModulePolarity();
        }
        else {
          // Kick off serial output here
          Ssc * sscptr = SSC;
          sscptr->SSC_TPR = CONVERT_PTR_UINT(extmodulePulsesData.pxx.getData());
          sscptr->SSC_TCR = extmodulePulsesData.pxx.getSize();
          sscptr->SSC_PTCR = SSC_PTCR_TXTEN;        // Start transfers
        }
        break;

      case PROTOCOL_CHANNELS_DSM2_LP45:
      case PROTOCOL_CHANNELS_DSM2_DSM2:
      case PROTOCOL_CHANNELS_DSM2_DSMX:
        // Alternate periods of 19.5mS and 2.5 mS
        period = pwmptr->PWM_CH_NUM[3].PWM_CPDR;
        if (period == 2500 * 2) {
          period = 19500 * 2;
        }
        else {
          period = 2500 * 2;
        }
        pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period; // Period in half uS
        if (period != 2500 * 2) {
          setupPulsesExternalModule();
        }
        else {
          // Kick off serial output here
          Ssc * sscptr = SSC;
          sscptr->SSC_TPR = CONVERT_PTR_UINT(extmodulePulsesData.dsm2.pulses);
          sscptr->SSC_TCR = (uint8_t *)extmodulePulsesData.dsm2.ptr - (uint8_t *)extmodulePulsesData.dsm2.pulses;
          sscptr->SSC_PTCR = SSC_PTCR_TXTEN;        // Start transfers
        }
        break;

#if defined(MULTIMODULE)
      case PROTOCOL_CHANNELS_MULTIMODULE:
#endif
      case PROTOCOL_CHANNELS_SBUS:
        // Todo: how to do inverted polarity on this platform?
        // Alternate periods of 5.5mS and 3.5 mS
        period = pwmptr->PWM_CH_NUM[3].PWM_CPDR;
        if (period == 3500 * 2) {
          period = 5500 * 2;
        }
        else {
          period = 3500 * 2;
        }
        pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = period; // Period in half uS
        if (period != 3500 * 2) {
          setupPulsesExternalModule();
        }
        else {
          // Kick off serial output here
          Ssc * sscptr = SSC;
          sscptr->SSC_TPR = CONVERT_PTR_UINT(extmodulePulsesData.dsm2.pulses);
          sscptr->SSC_TCR = (uint8_t *)extmodulePulsesData.dsm2.ptr - (uint8_t *)extmodulePulsesData.dsm2.pulses;
          sscptr->SSC_PTCR = SSC_PTCR_TXTEN;        // Start transfers
        }
        break;

      default:
        pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = *extmodulePulsesData.ppm.ptr++;
        if (*extmodulePulsesData.ppm.ptr == 0) {
          setExternalModulePolarity();
          setupPulsesExternalModule();
        }
        break;

    }
  }

  if (reason & PWM_ISR1_CHID1) {
    // TODO EXTRA_MODULE will be broken
    /*pwmptr->PWM_CH_NUM[1].PWM_CPDRUPD = *modulePulsesData[EXTRA_MODULE].ppm.ptr++;
    if (*modulePulsesData[EXTRA_MODULE].ppm.ptr == 0) {
      setupPulses(EXTRA_MODULE);
      setExtraModulePolarity();
    }*/
  }
}
#endif
