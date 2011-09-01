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

#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_pinsel.h"

#include	"usb.h"
#include	"sbl_iap.h"
#include	"sbl_config.h"
#include	"timer.h"
#include	"config.h"
#include	"debug.h"
#include	"buzzer.h"

#include	"soupcup32.h"

int main(void)
{
	// DeInit NVIC and SCBNVIC
	NVIC_DeInit();
	NVIC_SCBDeInit();

	/* Configure the NVIC Preemption Priority Bits:
	 * two (2) bits of preemption priority, six (6) bits of sub-priority.
	 * Since the Number of Bits used for Priority Levels is five (5), so the
	 * actual bit number of sub-priority is three (3)
	 */
	NVIC_SetPriorityGrouping(0x05);

	/* Change the Vector Table to the USER_FLASH_START
		in case the user application uses interrupts */
	SCB->VTOR = (USER_FLASH_START & 0x1FFFFF80);

	pins_init();

	// debug_num(1);

	// buzzer_play(2771, 50);

	SysTickTimer_Init(); // Initialize the timer for millis()

	buzzer_init();

	//buzzer_play(2000, 50);

	USBSerial_Init(); // Initialize USB<->Serial

	//buzzer_play(1000, 100);

	//debug_next();

	buzzer_play(2000, 50);
	buzzer_wait();
	buzzer_play(1775, 100);

 	soupcup32();

	NVIC_SystemReset();
}