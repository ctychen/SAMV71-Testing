#include <atmel_start.h>
#include <stdio.h>
#include "tc_lite.h"
#include "hal_gpio.h"

bool read_trig_channel(const uint8_t ch)
{
	bool status = gpio_get_pin_level(ch);
	// printf("trigger status: %s", status ? "true" : "false"); // need to set up serial for this to work
	return status;
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	//delay_init(TC2);
	
	//output has been changed to be with TC1, on PC30, which is TIOB5
	delay_init(TC1);
	
	// TC channel 0: input - external trigger 
	// TC channel 1: NEW -  output - generating PWM waveform 
	
	/* Replace with your application code */
	while (1) {
		// start channel if detection
		bool ch_status = read_trig_channel(PB3_TRIG);
		if (ch_status) 
		{
			//start_timer(TC2, 2);
			start_timer(TC1, 1);
		}
		else {
			//stop_timer(TC2, 2);
			stop_timer(TC1, 1);
		}
		//delay_ms(100); // Testing only 
	}
}