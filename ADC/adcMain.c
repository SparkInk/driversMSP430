
/**
 * main.c
 */

#include <msp430.h>
#include <stdio.h>
#include "usciUart.h"
#include "adc12.h"
#include "pwm.h"
#include "viperCmd.h"
#include "timerA0.h"
#include "vnh7070API.h"

int main(void){
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	volatile unsigned int buffer[300];
    volatile unsigned int sample = 0;
    volatile unsigned int adcToDuty;
	char result[15];

	// Initialise Uart
	usciA1UartInit();

	// Initialise PWM
	initPWM();

	adc12Cfg("3V3", 1, 1, 0);

	// Initialise timer
	timerA0Init();

	// CW direction
	vnh7070InputCtrl(1);
	P8DIR |= BIT2;
	P8OUT &= ~BIT2;
	update = 0;
    // enable the global interrupt
    __enable_interrupt();


    while (1) {
        // just do it

        if (update) {
            P8OUT ^= BIT2;
            // to update the sampling time
            setSampleTime(mem1Result);
            // to update the PWM signal
            adcToDuty = (100 * (long int)mem0Result) >> 12;
            setPWM(adcToDuty, 500);

            sample++;
            update = 0;
        }
    }

	return 0;
}


/**************************************
 *                                    *
 * Timer to update the voltage values *
 *                                    *
 **************************************/
#pragma vector = TIMER1_A0_VECTOR
__interrupt void timerA0ISR(void) {

    adc12SampSWConv();

}
