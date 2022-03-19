
#include <msp430.h>
#include <math.h>
#include "usciI2C.h"
#include "pixyCam2.h"
/*************************************************
 * pixyCam2 module for PixyCam v2
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 28.02.2022 (dd::mm::yyyy)
 * Date Modified: 1.03.2022 (dd::mm::yyyy)

 *
 * ************************************************/


/*********************************************************************
 * Name: pixyGetVersion 
 * 
 * Description: 
 * 
 * Arguments: slvAddr   -   a slave's address
 *            rxBuffer  -   a pointer to a buffer where a 
 *                          received data will be stored
 * 
 * Return: 0 if successful was a transaction; -1 if unsuccessful
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 28.02.2022 (dd::mm::yyyy)
 * Date Modified: 28.02.2022 (dd::mm::yyyy)
 * ******************************************************************/
char pixyGetVersion (unsigned char slvAddr, unsigned char *rxBuffer) {
    signed char result = -1;
    unsigned char tempRxBuf[12];
    unsigned char getCharReq[4] = {0xae, 0xc1, 0xe, 0x00};
    unsigned char rxCounter = 0;

    result = usciB0MstTx(4, PIXY_ADDR, getCharReq);
    result = usciB0MstRx(12, PIXY_ADDR, tempRxBuf);

    for (rxCounter; (rxCounter + 6) < 12; rxCounter++) {
        rxBuffer[rxCounter] = tempRxBuf[rxCounter + 6];
    }

    return result;
}
/*********************************************************************
 * Name: pixyGetVersion 
 * 
 * Description: 
 * 
 * Arguments: slvAddr   -   a slave's address
 *            rxBuffer  -   a pointer to a buffer where a 
 *                          received data will be stored
 * 
 * Return: 0 if successful was a transaction; -1 if unsuccessful
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 28.02.2022 (dd::mm::yyyy)
 * Date Modified: 28.02.2022 (dd::mm::yyyy)
 * ******************************************************************/
char pixyGetVector (unsigned char *rxBuffer) {
    signed char result = -1;
    unsigned char tempRxBuffer[64];
    unsigned char getMainFeaturesReq[6] = {0xae, 0xc1, 48, 2, 0, 1};    // byte[4] -> req. type: 0=main features, 1=all features; 
                                                                        // byte[5] -> features(bitmap): 0x0=vector, 0x2=intersections, 0x4=barcodes
    unsigned char rxCounter = 0;

    // issue a getMainFeatures request
    result = usciB0MstTx(6, PIXY_ADDR, getMainFeaturesReq);
    // receivea a getMainFeatures response
    result = usciB0MstRx(64, PIXY_ADDR, tempRxBuffer);
    // store vector's coordinates in rxBuffer
    for (rxCounter; rxCounter < 4; rxCounter++) {
        rxBuffer[rxCounter] = tempRxBuffer[rxCounter + 9];
    }
    
    return result;
}

/*********************************************************************
 * Name: computePixyAngle
 *
 * Description:
 *
 * Arguments:   x0, y0  -   coordinates of the tail of a vector
 *              x1, y1  -   coordinates of the head of a vector
 *
 * Return: angle in degrees
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 01.03.2022 (dd::mm::yyyy)
 * Date Modified: 01.03.2022 (dd::mm::yyyy)
 * ******************************************************************/

double computePixyAngle (unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1) {

    double dx = x1 - x0;
    double dy = y1 - y0;

    dx = atan2(dy, dx);

    dx = (dx * 180.0) / PI;

    return dx;
}
/************************************************************************************
* Function: dispCnsl
* Description: displays computedAngle on a console;
*              Serves as a replacement to sprintf(), because the latter refuses to
*              fulfill its duties :(
*
* Arguments: computedAngle  -   a number of counts
*
* return: none
* Author: Iakov Umrikhin
* Date Created: 06.02.2022 (dd:mm:yyyy)
* Date Modified: 1.03.2022 (dd::mm::yyyy)
************************************************************************************/
void dispCnsl(signed int computedAngle) {
    char buffer[8] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    if (computedAngle >= 0) {    // positive counts
        while (computedAngle > 0) {
            volatile int mod;
            mod = computedAngle % 10;
            buffer[2-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            computedAngle /= 10;
            j++;
        }
        for (i = (3-j); i < 3; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }
    if (computedAngle  < 0) {   // negative counts
        usciA1UartTxChar('-');  // display a negative sign
        computedAngle *= -1;    // convert from negative to positive
        while (computedAngle > 0) {
            volatile int mod;
            mod = computedAngle % 10;   // chunk off the last digit
            buffer[2-j] = mod;    // 0th element equals to the last digit of a number
            //usciA1UartTxChar(mod + '0');
            computedAngle /= 10;      // remove the last digit from the current number
            j++;
        }
        for (i = (3-j); i < 3; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 8);   // clear buffer from previous data
}

/************************************************************************************
* Function: dispCnsl
* Description: displays computedAngle on a console;
*              Serves as a replacement to sprintf(), because the latter refuses to
*              fulfill its duties :(
*
* Arguments: computedAngle  -   a number of counts
*
* return: none
* Author: Iakov Umrikhin
* Date Created: 06.02.2022 (dd:mm:yyyy)
* Date Modified: 1.03.2022 (dd::mm::yyyy)
************************************************************************************/
void dispCoord(signed int computedAngle) {
    char buffer[8] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    if (computedAngle >= 0) {    // positive counts
        while (computedAngle > 0) {
            volatile int mod;
            mod = computedAngle % 10;
            buffer[2-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            computedAngle /= 10;
            j++;
        }
        for (i = (3-j); i < 3; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 8);   // clear buffer from previous data
}

/************************************************************************************
* Function: dispCnsl
* Description: displays computedAngle on a console;
*              Serves as a replacement to sprintf(), because the latter refuses to
*              fulfill its duties :(
*
* Arguments: computedAngle  -   a number of counts
*
* return: none
* Author: Iakov Umrikhin
* Date Created: 06.02.2022 (dd:mm:yyyy)
* Date Modified: 1.03.2022 (dd::mm::yyyy)
************************************************************************************/
void dispVersion(long long int version) {
    char buffer[40] = 0;
    unsigned char i = 0;
    unsigned char j = 0;

    if (version >= 0) {    // positive counts
        while (version > 0) {
            volatile int mod;
            mod = version % 10;
            buffer[39-j] = mod;    //0th element equals to the last digit of a number
            //usciA1UartTxChar('0' + mod);    // shift a digit by 40 in ASCII table
            version /= 10;
            j++;
        }
        for (i = (40-j); i < 40; i++){
            usciA1UartTxChar('0' + buffer[i]);
        }
        i = 0;
        j = 0;
        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');
    }

    memset(buffer, 0, 8);   // clear buffer from previous data
}
