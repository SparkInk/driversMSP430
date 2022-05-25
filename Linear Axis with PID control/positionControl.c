/************************************************************
 *  posCtrl.c
 *
 *  This modules is to compute the PID controller output.
 *
 *  Author: Iakov (aka Iasha) Umrikhin
 *  Date Created: 09.05.2022 (dd.mm.yyyy)
 *  Date Modified: 09.05.2022 (dd.mm.yyyy)
 ************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <math.h>
#include "positionControl.h"

// Global variables
volatile unsigned char pidChange;
volatile unsigned char pidChangeStop;

ARROWS arrowKey;

/************************************************************************************
* Function: pidCompute
*
* Description: computes the PID values for the control loop
*
* Arguments: currAngle - current robot's angle
*
* Return: pidResult - a combined result of P, I, and D components
*
* Author: Iakov (aka Iasha) Umrikhin
* Date: 22.04.2022 (dd.mm.yyyy)
* Modified: 08.05.2022 (dd.mm.yyyy)
************************************************************************************/

unsigned int posCtrl(signed long int posCount, signed long int setPoint, PID *pidGain, signed char *motorDirection) {

    volatile signed int error;
    volatile static signed int prevError = 0;
    volatile static signed int aforePrevError = 0;
    volatile float dT = 0.1;    // time to integrate over is 100 ms
    volatile static unsigned char oneShot = 1;
    volatile signed int currPoint;

    // PID components
    volatile signed int pComp;
    volatile signed int iComp = 0;
    volatile signed int dComp;

    volatile signed int pidResult;
    volatile static signed int pidResultOld = 0;

    if (oneShot){
//        gains for the bench motor #E
//        pidGain->Kp = 0.08;
//        pidGain->Ki = 0.20;
//        pidGain->Kd = 0.004;
//        gains for the linear axis
          pidGain->Kp = 0.05;
          pidGain->Ki = 0.10;
          pidGain->Kd = 0.003;
        oneShot = 0;
    }

    // if one of the keys is pressed modify PID gains
    if (pidChange){
        changePID(pidGain);
        pidChange = 0;
    }
//
//    currPoint = posCount / 13;

    // calculate the error
    error = setPoint - posCount;

    // compute gains
    // proportional gain
    pComp = (error - prevError);
    

    // integral gain
    iComp = (error + prevError) >> 1;
    // iComp = constrain(iComp, -200, 200);
    
    // derivative gain; save the current angle value
    dComp = (error - 2 * prevError + aforePrevError) / dT;

    // compute the PID output result
    pidResult = pidResultOld + (pidGain->Kp * pComp + pidGain->Ki * iComp * dT + pidGain->Kd * dComp);

    aforePrevError = prevError;
    prevError = error;
    pidResultOld = pidResult;

    // constrain the PID output between 0-100 %
    *motorDirection = (pidResult > 0) ? 1 : 0;
    
    pidResult = constrain(abs(pidResult), 0, 100);

//    if (pidResult == 0) {
//        prevError = 0;
//        aforePrevError = 0;
//        pidResultOld = 0;
//    }
    // enable interrupts
    UCA1IE |= UCRXIE;

    return pidResult;
 }

/***************************************************************************************
* Name: changePID
* 
* Description: changes PID gains according to which key on the numpad is pressed
*
* 
* Arguments: arrowKey - structure containing bools for arrow keys
* 
* Return: You kid me not! Nothing in return
* 
* Author: Iakov (aka Iasha) Umrikhin
* 
* Date Created: 08.05.2022 (dd::mm::yyyy)
* Date Modified: 08.05.2022 (dd::mm::yyyy)
* 
* ************************************************************************************/
void changePID(PID *pid) {

    volatile char pidBuff[50];
    volatile signed int kpOld, kiOld, kdOld;

    clearArrowKeys(&arrowKey);

    // scale up to 100th
    pid->Kp *= 100;
    pid->Ki *= 100;
    pid->Kd *= 1000;

    kpOld = pid->Kp;
    kiOld = pid->Ki;
    kdOld = pid->Kd;

    // if the RIGHT key is pressed - right bit shift; if the LEFT key is pressed - left bit shift
    // else - don't change the bit's position

    // print a new line to separate a current routine from the previous ones
    newLine();
    usciA1UartTxString("Choose a gain to tune: 1 - Kp; 2 - Ki; 3 - Kd \r\n");
    newLine();

    while (!pidChangeStop) {     // this is active until the letter S is not pressed

        // enable interrupts
        UCA1IE |= UCRXIE;

        switch (arrowKey.bitPID){
        case 1:     // change Kp gain
            pid->Kp = arrowKey.up ? (pid->Kp + 1) : arrowKey.down ? (pid->Kp - 1) : pid->Kp;
            sprintf(pidBuff, "Kp(x0.01):   %.0f      \r\n", pid->Kp);
            usciA1UartTxString(pidBuff);

            // allows changing Kp only by 5; basically, a lazy debounce
            if (abs(kpOld - pid->Kp) >= 1) {
                clearArrowKeys(&arrowKey);
                kpOld = pid->Kp;
            }
            break;
        case 2:     // change Ki gain
            pid->Ki = arrowKey.up ? (pid->Ki + 1) : arrowKey.down ? (pid->Ki - 1) : pid->Ki;

            sprintf(pidBuff, "Ki(x0.01):  %.0f      \r\n", pid->Ki);
            usciA1UartTxString(pidBuff);

            // allows changing Ki only by 5; basically, a lazy debounce
            if (abs(kiOld - pid->Ki) >= 1) {
                clearArrowKeys(&arrowKey);
                kiOld = pid->Ki;
            }
            break;
        case 3:     // change Kd gain
            pid->Kd = arrowKey.up ? (pid->Kd + 1): arrowKey.down ? (pid->Kd - 1) : pid->Kd;
            
            sprintf(pidBuff, "Kd (x0.01):   %.0f      \r\n", pid->Kd);
            usciA1UartTxString(pidBuff);

            // allows changing Kd only by 5; basically, a lazy debounce
            if (abs(kdOld - pid->Kd) >= 1) {
                clearArrowKeys(&arrowKey);
                kdOld = pid->Kd;
            }
            break;
        }

    }

    // scale back to 0.01
    pid->Kp /= 100;
    pid->Ki /= 100;
    pid->Kd /= 1000;

    arrowKey.bitPID = 0;
    pidChangeStop = 0;

    // print out the result of PID change
    sprintf(pidBuff, "Kp:   %.2f   Ki:    %.2f  Kd:     %.3f  \r\n", pid->Kp, pid->Ki, pid->Kd);
    usciA1UartTxString(pidBuff);
    newLine();
}
/***************************************************************************************
* Name: mtrDirUpdate()
* 
* Description: 
*
* 
* Arguments: 
* 
* Return: You kid me not! Nothing in return
* 
* Author: Iakov (aka Iasha) Umrikhin
* 
* Date Created: 15.05.2022 (dd::mm::yyyy)
* Date Modified: 15.05.2022 (dd::mm::yyyy)
* 
* ************************************************************************************/
void mtrDirUpdate(unsigned char direction, unsigned int dutyCycle) {

    volatile char printStr[50];
    volatile signed char status;

    // update motor's direction
    if (!direction) {    // CCW mode

        status = vnh7070CW(dutyCycle);
        // valid or invalid command
/*        if (!status) {
            sprintf(printStr, "CCW mode is on\r\n");
        }
        else {
            sprintf(printStr, "Invalid command\r\n");
        }*/
    }
    else if (direction) {     // CW mode

        status = vnh7070CCW(dutyCycle);
        // valid or invalid command
/*        if (!status) {
            sprintf(printStr, "CW mode is on\r\n");
        }
        else {
            sprintf(printStr, "Invalid command\r\n");
        }*/
    }
/*    else {  // none of the modes
        sprintf(printStr, "Invalid command\r\n");
    }*/

    // print out the result
/*    usciA1UartTxString(printStr);
    newLine();
    newLine();*/

}
/***************************************************************************************
 * Name: clearArrowKeys
 * 
 * Description: clears whatever values each arrow-key variable currently has
 *
 * 
 * Arguments: arrowKey - structure containing bools for arrow keys
 * 
 * Return: You kid me not! Nothing in return
 * 
 * Author: Iakov (aka Iasha) Umrikhin
 * 
 * Date Created: 08.05.2022 (dd::mm::yyyy)
 * Date Modified: 08.05.2022 (dd::mm::yyyy)
 * 
 * ************************************************************************************/
void clearArrowKeys(ARROWS *arrow) {
    arrow->up = 0;
    arrow->down = 0;
}

/***************************************************************************************
 * Name: detectArrowKey
 *
 * Description: detects if one of the arrow keys has been pressed;
 *              if true, assigns 1 to a variable pertaining to the key
 *
 *
 * Arguments: arrowKey - structure containing variables for each arrow key
 *
 * Return: You kid me not! Nothing in return
 *
 * Author: Iakov (aka Iasha) Umrikhin
 *
 * Date Created: 08.05.2022 (dd::mm::yyyy)
 * Date Modified: 08.05.2022 (dd::mm::yyyy)
 *
 * ************************************************************************************/
void detectArrowKey(ARROWS *arrow) {

    switch(UCA1RXBUF){

        case 0x41:              // UP key is pressed
            arrow->up = 1;
            break;
        case 0x42:              // DOWN key is pressed
            arrow->down = 1;
            break;
        case 0x31:              // NUM_1 is pressed
            arrow->bitPID = 1;
            break;
        case 0x32:              // NUM_2 is pressed
            arrow->bitPID = 2;
            break;
        case 0x33:              // NUM_3 is pressed
            arrow->bitPID = 3;
            break;
    }

}

/***************************************************************************************
 * Name: constrain
 *
 * Description: constrains the input variable between high and low boundaries 
 *
 * Arguments: modifiedVar   -   the input variable to constrain
 * 
 *            lowBoundary   -   the low boundary 
 * 
 *            highBoundary  -   the high boundary 
 *
 * Return:  the input variable if it is within (lowBoundary, highBoundary);
 *          otherwise, hihgBoundary if the input is greater; lowBoundary if the input is lower.
 *
 * Author: Iakov (aka Iasha) Umrikhin
 *
 * Date Created: 28.04.2022 (dd::mm::yyyy)
 * Date Modified: 28.04.2022 (dd::mm::yyyy)
 *
 * ************************************************************************************/

signed int constrain(signed int modifiedVar, signed int lowBoundary, signed int highBoundary) {

    return (modifiedVar > highBoundary) ? highBoundary : (modifiedVar < lowBoundary) ? lowBoundary : modifiedVar;

}

/***************************************************************************************
 * Name: constrainl
 *
 * Description: constrains the input variable between high and low boundaries;
 *              solely for long signed integers
 *
 * Arguments: modifiedVar   -   the input variable to constrain
 *
 *            lowBoundary   -   the low boundary
 *
 *            highBoundary  -   the high boundary
 *
 * Return:  the input variable if it is within (lowBoundary, highBoundary);
 *          otherwise, hihgBoundary if the input is greater; lowBoundary if the input is lower.
 *
 * Author: Iakov (aka Iasha) Umrikhin
 *
 * Date Created: 28.04.2022 (dd::mm::yyyy)
 * Date Modified: 28.04.2022 (dd::mm::yyyy)
 *
 * ************************************************************************************/

long signed int constrainl(long signed int modifiedVar, long signed int lowBoundary, long signed int highBoundary) {

    return (modifiedVar > highBoundary) ? highBoundary : (modifiedVar < lowBoundary) ? lowBoundary : modifiedVar;

}
/*********************************
 *      UART Interrupt           *
 *                               *
 * Used to detect key presses *
 *********************************/

#pragma vector = USCI_A1_VECTOR
 __interrupt void uartISR (void) {

     // disable interrupts
     UCA1IE &= ~UCRXIE;

     switch(UCA1RXBUF) {
        case 0x63:              // letter 'c' is pressed
            executeCmdUpdate = 1;
            break;

        case 0x70:              // letter 'p' is pressed
            pidChange = 1;
            break;

        case 0x0d:              // ENTER key is pressed
            enterConsole = 1;   
            break;

        case 0x73:              // letter 's' is pressed
            pidChangeStop = 1;
            break;
     }

     detectArrowKey(&arrowKey);

     // clear interrupt flags
     UCA1IFG &= ~UCRXIFG;
 }
