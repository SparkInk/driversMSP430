

/**************************************************************************
 *                             mpu6050TestBench.c
 *
 * This test bench is for MPU6050 sensor.
 *
 * If the test is successful, then this bench will be used to test PixyCam
 *
 **************************************************************************/

#include <msp430.h>
#include "usciI2C.h"
#include "pixyCam2.h"


int main(void)

{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	unsigned char verStr[GET_VERSION_VER_NUM_BYTES];
	unsigned char vectorStr[VECTOR_NUM_BYTES];
	usciB0I2CInit(SMCLK_DIV_10);

	UCB0IFG &= ~UCNACKIFG;
	UCB0IE |= UCNACKIE;
	__enable_interrupt();


	while (1) {
	    pixyGetVersion(PIXY_ADDR, verStr);
		__no_operation();
		pixyGetVector(vectorStr);
		__no_operation();
	}

	return 0;
}

