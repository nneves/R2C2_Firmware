/* Copyright (C) 2009-2010 Michael Moon aka Triffid_Hunter   */
/* Copyright (c) 2011 Jorge Pinto - casainho@gmail.com       */
/* All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "temp.h"
#include "machine.h"
#include "pinout.h"
#include "serial.h"
#include "sermsg.h"
#include "dda.h"
#include "sersendf.h"
#include "debug.h"
#include "ios.h"

/* Table for NTC EPCOS B57560G104F and R1 = 330R for Extruder0
 * Table for NTC EPCOS B57560G104F and R1 = 12K for HeatedBed0 */
const uint16_t temptable[NUMTEMPS][3] = {
  {1009,   36, 300}, /* {ADC value Extruder0, ADC value HeatedBed0, temperature} */
  {1119,   42, 290},
  {1240,   48, 280},
  {1372,   56, 270},
  {1517,   65, 260},
  {1673,   76, 250},
  {1839,   90, 240},
  {2015,  106, 230},
  {2198,  126, 220},
  {2385,  151, 210},
  {2573,  182, 200},
  {2759,  220, 190},
  {2940,  268, 180},
  {3112,  328, 170},
  {3270,  402, 160},
  {3415,  496, 150},
  {3544,  614, 140},
  {3655,  761, 130},
  {3750,  941, 120},
  {3830, 1161, 110},
  {3894, 1420, 100},
  {3946, 1719,  90},
  {3986, 2048,  80},
  {4017, 2394,  70},
  {4041, 2737,  60},
  {4058, 3056,  50},
  {4070, 3335,  40},
  {4079, 3563,  30},
  {4085, 3738,  20},
  {4089, 3866,  10},
  {4092, 3954,   0}
};

uint16_t current_temp[NUMBER_OF_SENSORS] = {0, 0}, target_temp[NUMBER_OF_SENSORS] = {0, 0};

#ifndef	ABSDELTA
#define	ABSDELTA(a, b)	(((a) >= (b))?((a) - (b)):((b) - (a)))
#endif

uint16_t temp_read(uint8_t sensor_number)
{
  return current_temp[sensor_number];
}

void temp_set(uint16_t t, uint8_t sensor_number)
{
  if (t)
  {
    steptimeout = 0;
    power_on();
  }

  target_temp[sensor_number] = t;
}

uint16_t temp_get(uint8_t sensor_number)
{
  return current_temp[sensor_number];
}

uint16_t temp_get_target(uint8_t sensor_number)
{
  return target_temp[sensor_number];
}

uint8_t	temp_achieved(uint8_t sensor_number)
{
  if (current_temp[sensor_number] >= (target_temp[sensor_number] - 2))
    return 255;

  return 0;
}

void temp_print()
{
  sersendf("ok T:%u.0 B:%u.0\r\n", current_temp[EXTRUDER_0], current_temp[HEATED_BED_0]); /* for RepRap software */
}

void temp_tick(void)
{

/* Define a value for sequencial number of reads of ADC, to average the readed
 * value and try filter high frequency noise.
 */
#define ADC_READ_TIMES 4

  /* Read and average the ADC input signal */
  current_temp[EXTRUDER_0] = 0;
  for (uint8_t c = 0; c < ADC_READ_TIMES; c++)
  {
    /* Read EXTRUDER_0 temperature value and manage heater */
    current_temp[EXTRUDER_0] += read_temp(EXTRUDER_0);
  }
  current_temp[EXTRUDER_0] = current_temp[EXTRUDER_0] / ADC_READ_TIMES;

  /* Manage heater using simple ON/OFF logic, no PID */
  if (current_temp[EXTRUDER_0] < target_temp[EXTRUDER_0])
  {
    extruder_heater_on();
  }
  else
  {
    extruder_heater_off();
  }


  /* Read and average the ADC input signal */
  current_temp[HEATED_BED_0] = 0;
  for (uint8_t c = 0; c < ADC_READ_TIMES; c++)
  {
    /* Read HEATED_BED_0 temperature value and manage heater */
    current_temp[HEATED_BED_0] += read_temp(HEATED_BED_0);
  }
  current_temp[HEATED_BED_0] = current_temp[HEATED_BED_0] / ADC_READ_TIMES;

  /* Manage heater using simple ON/OFF logic, no PID */
  if (current_temp[HEATED_BED_0] < target_temp[HEATED_BED_0])
  {
    heated_bed_on();
  }
  else
  {
    heated_bed_off();
  }
}

uint16_t read_temp(uint8_t sensor_number)
{
  uint16_t raw, celsius;
  uint8_t i;

  if (sensor_number == EXTRUDER_0)
  {
    raw = analog_read(EXTRUDER_0_SENSOR_ADC_CHANNEL);
  }
  else if (sensor_number == HEATED_BED_0)
  {
    raw = analog_read(HEATED_BED_0_SENSOR_ADC_CHANNEL);
  }

  /* Go and use the temperature table to math the temperature value... */
  if (raw < temptable[0][sensor_number]) /* Limit the smaller value... */
  {
    celsius = temptable[0][2];
  }
  else if (raw >= temptable[NUMTEMPS-1][sensor_number]) /* Limit the higher value... */
  {
    celsius = temptable[NUMTEMPS-1][2];
  }
  else
  {
    for (i=1; i<NUMTEMPS; i++)
    {
      if (raw < temptable[i][sensor_number])
      {
        celsius = temptable[i-1][2] +
            (raw - temptable[i-1][sensor_number]) *
            (temptable[i][2] - temptable[i-1][2]) /
            (temptable[i][sensor_number] - temptable[i-1][sensor_number]);

        break;
      }
    }
  }

  return celsius;
}