
/********************************************************
 * 
 * DE-ACCM2G Buffered +-2g Accelerometer main module
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 02.05.2022
 * Date Modified: 02.05.2022
 ********************************************************/

#include <msp430.h> 
#include <stdio.h>
#include "incSensor.h"
#include "adc12.h"
#include "usciUart.h"
#include "timerA0.h"

int main(void) {

	volatile signed int angleAvg;
	volatile signed int angleReal;

	volatile char outBuffer[50];

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// set up the uart 
	usciA1UartInit();

	// set up the adc
	adc12Cfg("2V5", 1, 1, 0);

	// set up the timer
	timerA0Init();

	__enable_interrupt();

	while (1) {

		if (update) {

			// compute the average inclination angle
			angleAvg = angleAvgCompute(mem0Result, mem1Result);

			update = 0;
		}
		
		// compute the real inclination angle
		angleReal = angleRealCompute(mem0Result, mem1Result);

		sprintf(outBuffer, "Angle (real): %d, angle (average): %d \r\n", angleReal, angleAvg);
		usciA1UartTxString(outBuffer);

	}

	return 0;
}
