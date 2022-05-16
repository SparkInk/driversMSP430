/******************************************************************
 *                Header file for pvtmMain.c                      *
 * Author: Iakov Umrikhin
 * Date Created: 06.03.2022 (dd:mm:yyyy)
 * Date Modified: 06.03.2022 (dd:mm:yyyy)
 * ****************************************************************/


#include <msp430.h>

#ifndef TIMERA0_H
#define TIMERA0_H

    // Macros and constants

    #define DISP_UPDATE_RATE    10000
    #define SMCLK_FRQ           1000000

    // global variables
    volatile unsigned char displayRPM;

    typedef struct ENC_STATE {
        unsigned char chAState;
        unsigned char chBState;
    }ENC_STATE;

    // functions prototypes
    void timerA0Init();

    void timerA1Init();

    unsigned char fwDirection(ENC_STATE encState);
    void setSampleTime(unsigned int adcResult);

#endif
