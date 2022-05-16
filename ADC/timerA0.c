/*
 * timerA0.c
 *
 *  Created on: Apr 4, 2022
 *      Author: iumrikhin
 */



/******************************************************************
 * Name: timerA0Init
 *
 * Description:
 *
 *
 * Arguments:
 *
 * Return: none
 *
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 ******************************************************************/
#include <msp430.h>
#include <math.h>
#include "adc12.h"

// timer needs a fix
void timerA0Init() {

    TA0CTL |= TASSEL_1 | ID_3 | MC_1 | TACLR;   // ACLK; Input divider 8; Up mode

    TA0EX0 |= TAIDEX_7;                         // division by 8

    TA0CCR0 = 11;  // trigger every 100 samples
    TA0CCR1 = 0.1 * TA0CCR0;   // duty cycle is 10%

    // TA0CCTL1 initialisation
    TA0CCTL1 |= OUTMOD_3;

//    // enable interrupt on CCR1
//    TA0CCTL0 |= CCIE;

    // ports initialisation; P1.2: Timer_A0 CCI1B; CBOUT; TA0CCTL1; CCR1; CCIS |= BIT1
    P1DIR |= BIT2;           // P1.2 is 1
    P1SEL |= BIT2;          // select CCI1B


    // clear all pending interrupt flags
    TA0CCTL1 &= ~CCIFG;
    TA0CTL &= ~TAIFG;

}

/******************************************************************
 * Name: setSampleTime
 *
 * Description:
 *
 *
 * Arguments:
 *
 *
 *
 *
 * Return: none
 *
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 ******************************************************************/

void setSampleTime(unsigned int adcResult) {


    if (adcResult <= 10) {   // f = 50 Hz
        TA0CCR0 = 2;
    }
    else if (adcResult <= 30){  // f = 25 Hz
        TA0CCR0 = 6;
    }
    else if (adcResult <= 100){ //f = 5 Hz
        TA0CCR0 = 35;
    }
    else if (adcResult <= 200){ // f = 2.5 Hz
        TA0CCR0 = 68;
    }
    else if (adcResult <= 500){  // f = 1 Hz
        TA0CCR0 = 141;
    }
    else if (adcResult <= 1000){ // f = .5 Hz
        TA0CCR0 = 351;
    }
    else if (adcResult <= 2000) { // f = .25 Hz
        TA0CCR0 = 701;
    }
    else if(adcResult <= 4000) {
        TA0CCR0 = 1401;
    }

    TA0CCR1 = 1;   // duty cycle is 10%
}
