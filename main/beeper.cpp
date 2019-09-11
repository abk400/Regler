#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/sens_reg.h"

#include "driver/gpio.h"

#include "driver/dac.h"

#include "beeper.h"

static const dac_channel_t channel = DAC_CHANNEL_2; // GPIO26

Beeper::Beeper() {
  int clk_8m_div = 0;
  int scale = 0;
  int offset = 0;
  int invert = 2;
  int frequency_step =
      (int)(m_freq * 65536 * (1 + clk_8m_div) / RTC_FAST_CLK_FREQ_APPROX);

  // Enable tone generator
  SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL1_REG, SENS_SW_TONE_EN);
  // Enable / connect tone tone generator
  SET_PERI_REG_MASK(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_CW_EN2_M);

  // Scale
  //  * - 00: no scale
  //  * - 01: scale to 1/2
  //  * - 10: scale to 1/4
  //  * - 11: scale to 1/8
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_SCALE2, scale,
                    SENS_DAC_SCALE2_S);
  // Offset
  //  Range 0x00 - 0xFF
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_DC2, offset,
                    SENS_DAC_DC2_S);
  // Invert
  //  * - 00: does not invert any bits,
  //  * - 01: inverts all bits,
  //  * - 10: inverts MSB,
  //  * - 11: inverts all bits except for MSB
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL2_REG, SENS_DAC_INV2, invert,
                    SENS_DAC_INV2_S);

  //
  REG_SET_FIELD(RTC_CNTL_CLK_CONF_REG, RTC_CNTL_CK8M_DIV_SEL, clk_8m_div);
  SET_PERI_REG_BITS(SENS_SAR_DAC_CTRL1_REG, SENS_SW_FSTEP, frequency_step,
                    SENS_SW_FSTEP_S);

  m_isOn = false;
  // start();
}

void Beeper::start() {
  // Enable output
  if (!m_isOn) {
    dac_output_enable(channel);
    m_isOn = true;
  }
}
void Beeper::stop() {
  // Disable output
  if (m_isOn) {
    dac_output_disable(channel);
    m_isOn = false;
  }
}

int Beeper::getBeeperTimeout() { return m_timeout; }

bool Beeper::isOn() {
    return m_isOn;
}