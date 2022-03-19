/*
 * angleDisp.c
 *  this source file is for all the functions needed to
 *  display an angle on LCD received from the motor
 *
 *  Created on: Feb. 11, 2022
 *      Author: Iakov Umrikhin
 */
#include <msp430.h>
#include <math.h>
#include "angleDisp.h"
#include "usciUart.h"
#include "nok5110LCD.h"

// global variables
volatile signed int angleOffset;

/************************************************************************************
* Function: displayCounts
* Description: displays posCounts on a console;
*              Serves as a replacement to sprintf()
*
* Arguments: posCounts  -   a number of counts
*
* return: none
* Author: Iakov Umrikhin
* Date: 06.02.2022 (dd:mm:yyyy)
* Modified: 11.02.2022 (dd:mm:yyyy)
************************************************************************************/
void displayCounts(signed long int posCounts) {
    char buffer[8] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    if (posCounts >= 0) {    // positive counts
        while (posCounts > 0) {
            volatile int mod;
            mod = posCounts % 10;
            buffer[7-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            posCounts /= 10;
            j++;
        }
        for (i = (8-j); i < 8; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        // usciA1UartTxChar('\r');
        // usciA1UartTxChar('\n');
    }
    if (posCounts  < 0) {   // negative counts
        usciA1UartTxChar('-');  // display a negative sign
        posCounts *= -1;    // convert from negative to positive
        while (posCounts > 0) {
            volatile int mod;
            mod = posCounts % 10;   // chunk off the last digit
            buffer[7-j] = mod;    // 0th element equals to the last digit of a number
            //usciA1UartTxChar(mod + '0');
            posCounts /= 10;      // remove the last digit from the current number
            j++;
        }
        for (i = (8-j); i < 8; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        // usciA1UartTxChar('\r');
        // usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 4);   // clear buffer from previous data
}

/************************************************************************************
 * Function: dispAngleConsole
 *
 * Description: this function displays an angle (positive or negative) on the console
 *              instead of the counts, because the life is complicated enough
 *
 * Arguments:   posCounts   -   counts from the encoder
 *
 * Return:  none
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 11.02.2022 (dd:mm:yyyy)
 * Date Modified: 11.02.2022 (dd:mm:yyyy)
 ************************************************************************************/
void dispAngleConsole (signed long int posCounts){
    char buffer[8] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    posCounts /= CNT_TO_ANG;     // convert to angles

    if (posCounts >= 0) {    // positive counts
        while (posCounts > 0) {
            volatile int mod;
            mod = posCounts % 10;
            buffer[3-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            posCounts /= 10;
            j++;
        }
        for (i = (4-j); i < 4; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }
    if (posCounts  < 0) {   // negative counts
        usciA1UartTxChar('-');  // display a negative sign
        posCounts *= -1;    // convert from negative to positive
        while (posCounts > 0) {
            volatile int mod;
            mod = posCounts % 10;   // chunk off the last digit
            buffer[3-j] = mod;    // 0th element equals to the last digit of a number
            //usciA1UartTxChar(mod + '0');
            posCounts /= 10;      // remove the last digit from the current number
            j++;
        }
        for (i = (4-j); i < 4; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 4);   // clear buffer from previous data

}

/************************************************************************************
 * Function: dispAngleLCDline
 *
 * Description: this function rotates a single line clockwise, if posCounts > 0, and
 *                                                  counterclockwise, if posCounts > 0
 *
 * Arguments:   posCounts   -   counts from the encoder
 *
 * Return:  a number of revolutions
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 11.02.2022 (dd:mm:yyyy)
 * Date Modified: 14.02.2022 (dd:mm:yyyy)
 ************************************************************************************/

signed int dispAngleLCDline (signed long int posCounts) {

    volatile signed int revCount = 0;  // counter for revolutions
    volatile signed int angle = 0;
    volatile unsigned int newXi, newYi;
    volatile unsigned int newXf, newYf;
    // clear the display for the previous line
    nokLcdClear();

    // convertion from counts to an angle in degrees
    angle  = (posCounts  - angleOffset) / CNT_TO_ANG;

    // calculate a number of revolutions
    if (angle >= 0) {   // postive rotation
        revCount = angle / ONE_REV; // increment counter for revolutions
    }
    else if (angle < 0) {   // negative rotation
        revCount = angle / ONE_REV;
    }

    angle -= revCount * 360;    // change the scale to from 0 to 360

    // calculating new absolute initial coordinates; moving clockwise
    newXi = LCD_MAX_COL / 2 + abs(RADIUS * sin(RADTODEG * angle));  // 42
    newYi = RADIUS - abs(RADIUS * cos(RADTODEG * angle));       // 0

    // choose coordinates based on the quadrant they are on
    if (angle >= 0) {   // positive rotation; clockwise
        cwRotation(angle, newXi, newYi, newXf, newYf);
    }
    else if (angle < 0) {   // negative rotation; counterclockwise
        ccwRotation(angle, newXi, newYi, newXf, newYf);
    }

    return revCount;
}

/************************************************************************************
 * Function: dispAngleLCDbar
 *
 * Description: this function draws a horizontal bar on the display with respect to 
 *              the rotation: positive - the bar is being drawn from right to left, top left corner; 
 *                                       rows increment with each revolution from 0 to 6
 *                            negative - the bar is being drawn from left to right, bottom right corner; 
 *                                       rows decrement with each revolution from 6 to 0
 * Arguments:   posCounts   -   counts from the encoder
 *
 * Return:  a number of revolutions
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 11.02.2022 (dd:mm:yyyy)
 * Date Modified: 15.02.2022 (dd:mm:yyyy)
 ************************************************************************************/
signed int dispAngleLCDbar (signed long int posCounts) {
    signed int revCount = 0;  // counter for revolutions
    signed int angle = 0;
    unsigned int negBank = 0;
    unsigned int numCol, numColAbs = 0;
    unsigned int colCounter = 0;

    // clear the display for the previous line
    nokLcdClear();

    // convertion from counts to an angle in degrees and calculation of revolutions
    angle  = (posCounts - angleOffset) / (signed long int)CNT_TO_ANG;
    revCount = angle / (signed long int)ONE_REV; // increment counter for revolutions

    // calculate a number of revolutions
    if (angle >= 0) {   // postive rotation

        numCol = posCounts / UPDATE_RATE_BAR;
        // set initial coordinates
        nokLcdWrite(LCD_SET_XRAM, DC_CMD);  // set X coordinate at 0
        nokLcdWrite(LCD_SET_YRAM, DC_CMD);  // set Y coordinate at 0

        for (colCounter; colCounter < numCol; colCounter++) {   // drawing the bar
            nokLcdWrite(0xff, DC_DAT);
        }
    }
    else if (angle < 0) {   // negative rotation

        //compute a bank
        negBank = 5 - abs(revCount) % 6;

        // compute a number of colums to fill up
        numCol = abs(posCounts) / UPDATE_RATE_BAR;

        // compute an absolute value of the columns to set an x coordinate to
        numColAbs = numCol - abs(revCount) * LCD_MAX_COL;

        if (numColAbs >= LCD_MAX_COL) {
            numColAbs = (LCD_MAX_COL - 1);
        }

        if (numCol > LCD_MAX_COL * (6 - negBank)) {
            numCol = LCD_MAX_COL * (6 - negBank);
        }

        nokLcdWrite(LCD_SET_XRAM | (LCD_MAX_COL - numColAbs), DC_CMD);
        nokLcdWrite(LCD_SET_YRAM | negBank, DC_CMD);

        for (colCounter; colCounter < numCol; colCounter++) {
            nokLcdWrite(0xff, DC_DAT);
        }
        
    }

    // do I need this thing down below? I dunno
    angle -= revCount * 360;    // change the scale to from 0 to 360

    return revCount;
}

/************************************************************************************
 * Function: cwRotation(angle)
 *
 * Description: a shortcut for the clockwise rotations
 *
 * Arguments:  
 *
 * Return:  none
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 14.02.2022 (dd:mm:yyyy)
 * Date Modified: 14.02.2022 (dd:mm:yyyy)
 ************************************************************************************/
void cwRotation(signed int angle, unsigned int newXi, unsigned int newYi, unsigned int newXf, unsigned int newYf) {

    if (angle <= 90) {  //  second on the display (first quadrant mathematically)
        // initial coordinates stay the same
        // calculating new final coordinates
        newXf = LCD_MAX_COL - newXi;
        newYf = LCD_MAX_ROW - newYi;

    }
    else if ((angle > 90 && angle <= 180) || (angle > 270 && angle <= 360)) {  // third quadrant on the display (second mathematically)
        // for the final coordinates
        // final values
        newXf = newXi;
        newYf = LCD_MAX_ROW - newYi;
        //  newYi is the same as the absolute, but newXi is subtracted from 84
        newXi = LCD_MAX_COL - newXi;
    }
    else if (angle > 180 && angle <= 270) { // fourth quadrant on the display (third mathematically)
        // for this case the final coordinates will be the same as absolute coordinates
        newXf = newXi;  
        newYf = newYi;
        // however newXi and newYi need to be decremented by 84 and 48 respectively
        newXi = LCD_MAX_COL - newXi;
        newYi = LCD_MAX_ROW - newYi;
    }

    nokLcdDrawLine(newXi, newYi, newXf, newYf);
}
/************************************************************************************
 * Function: cwRotation(angle)
 *
 * Description: a shortcut for the counter-clockwise rotations
 *
 * Arguments:
 *
 * Return:  none
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 14.02.2022 (dd:mm:yyyy)
 * Date Modified: 14.02.2022 (dd:mm:yyyy)
 ************************************************************************************/
void ccwRotation(signed int angle, unsigned int newXi, unsigned int newYi, unsigned int newXf, unsigned int newYf) {

    if (angle >= -90 || (angle < -180 && angle >= -270)) {
        // for the final coordinates
        // final values
        newXf = newXi;
        newYf = LCD_MAX_ROW - newYi;
        //  newYi is the same as the absolute, but newXi is subtracted from 84
        newXi = LCD_MAX_COL - newXi;
    }
    else if (angle < -90 && angle >= -180) {
        // for this case the final coordinates will be the same as absolute coordinates
        newXf = newXi;  
        newYf = newYi;
        // however newXi and newYi need to be decremented by 84 and 48 respectively
        newXi = LCD_MAX_COL - newXi;
        newYi = LCD_MAX_ROW - newYi;
    }
    else if (angle < -270 && angle >= -360) {
        // initial coordinates stay the same
        // calculating new final coordinates
        newXf = LCD_MAX_COL - newXi;
        newYf = LCD_MAX_ROW - newYi;
    }

    nokLcdDrawLine(newXi, newYi, newXf, newYf);
}

