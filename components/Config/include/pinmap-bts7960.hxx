/**********************************************************************
ESP32 COMMAND STATION

COPYRIGHT (c) 2022 Mike Dunston

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses
**********************************************************************/

#ifndef HARDWARE_PINMAP_BTS7960_HXX_
#define HARDWARE_PINMAP_BTS7960_HXX_

#include "sdkconfig.h"

#undef CONFIG_DCC_TRACK_SIGNAL_PIN
#undef CONFIG_OPS_HBRIDGE_L298
#undef CONFIG_OPS_HBRIDGE_LMD18200
#undef CONFIG_OPS_HBRIDGE_DRV880X
#undef CONFIG_OPS_HBRIDGE_DRV8873
#undef CONFIG_OPS_HBRIDGE_DRV8873_5A
#undef CONFIG_OPS_HBRIDGE_POLOLU
#undef CONFIG_OPS_HBRIDGE_BTS7960B_5A
#undef CONFIG_OPS_HBRIDGE_BTS7960B_10A
#undef CONFIG_RAILCOM_TRIGGER_PIN
#undef CONFIG_RAILCOM_DATA_PIN
#undef CONFIG_RAILCOM_SHORT_PIN
#undef CONFIG_OLED_RESET_PIN
#undef CONFIG_PROG_TRACK_ENABLE_PIN
#undef CONFIG_RAILCOM_CUT_OUT_ENABLED
#undef CONFIG_RAILCOM_DATA_ENABLED
#undef CONFIG_OPSTRACK_ADC_CHANNEL_0
#undef CONFIG_OPSTRACK_ADC_CHANNEL_1
#undef CONFIG_OPSTRACK_ADC_CHANNEL_2
#undef CONFIG_OPSTRACK_ADC_CHANNEL_3
#undef CONFIG_OPSTRACK_ADC_CHANNEL_4
#undef CONFIG_OPSTRACK_ADC_CHANNEL_5
#undef CONFIG_OPSTRACK_ADC_CHANNEL_6
#undef CONFIG_OPSTRACK_ADC_CHANNEL_7
#undef CONFIG_PROGTRACK_ADC_CHANNEL_0
#undef CONFIG_PROGTRACK_ADC_CHANNEL_1
#undef CONFIG_PROGTRACK_ADC_CHANNEL_2
#undef CONFIG_PROGTRACK_ADC_CHANNEL_3
#undef CONFIG_PROGTRACK_ADC_CHANNEL_4
#undef CONFIG_PROGTRACK_ADC_CHANNEL_5
#undef CONFIG_PROGTRACK_ADC_CHANNEL_6
#undef CONFIG_PROGTRACK_ADC_CHANNEL_7
#undef CONFIG_RAILCOM_CUT_OUT_ENABLED
#undef CONFIG_RAILCOM_DATA_ENABLED
#undef CONFIG_I2C_SCL_PIN
#undef CONFIG_I2C_SDA_PIN

#if CONFIG_ESP32CS_BTS7960B

    #define CONFIG_DCC_TRACK_SIGNAL_PIN 19

    #define CONFIG_OPS_HBRIDGE_BTS7960B_5A 1

    #define CONFIG_OPSTRACK_ADC_CHANNEL_0 1

    #define CONFIG_OPS_TRACK_ENABLE_PIN 25
    #define CONFIG_PROG_TRACK_ENABLE_PIN -1

    #undef CONFIG_PROG_TRACK_ENABLED

#elif CONFIG_ESP32CS_BTS7960B_X2

    #define CONFIG_DCC_TRACK_SIGNAL_PIN 19

    #define CONFIG_OPS_HBRIDGE_BTS7960B_5A 1

    #define CONFIG_PROG_HBRIDGE_BTS7960B_5A 1

    #define CONFIG_OPSTRACK_ADC_CHANNEL_0 1

    #define CONFIG_PROGTRACK_ADC_CHANNEL_3 1

    #define CONFIG_OPS_TRACK_ENABLE_PIN 25

    #define CONFIG_PROG_TRACK_ENABLE_PIN 23
#endif // H-BRIDGE AUTO-ASSIGNMENT

#define CONFIG_I2C_SCL_PIN 22

#define CONFIG_I2C_SDA_PIN 21

#endif // HARDWARE_PINMAP_BTS7960_HXX_