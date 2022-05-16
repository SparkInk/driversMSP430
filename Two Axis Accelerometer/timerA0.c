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

void timerA0Init() {

    TA0CTL |= TASSEL_1 | ID_3 | MC_1 | TACLR;   // ACLK; Input divider 8; Up mode

    // TA0EX0 |= TAIDEX_3;                         // division by 4

    TA0CCR0 = 15;  // trigger every 15 samples ~ 2 kHz
    TA0CCR1 = 3;   // duty cycle is 10%

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
