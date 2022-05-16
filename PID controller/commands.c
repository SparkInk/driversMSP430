

/**********************************************************************************
*                                commands.c
* 
* This module contains all the commands needed for the position control 
* of the DC motor through FEDI and VNH7070API, and PID controller.
*
* 
* Author: Iakov (aka Iasha) Umrikhin
* 
* Date Created: 15.05.2022 (dd.mm.yyyy)
* Date Modified: 15.05.2022 (dd.mm.yyyy)
*********************************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include "positionControl.h"
#include "pwm.h"
#include "commands.h"
#include "usciUart.h"
#include "LS7366R.h"
#include "vnh7070API.h"

#define SHIFT_8 256
#define SHIFT_16 65536
#define SHIFT_24 16777216

// macros for registers' names
#define MDR0_REG "MDR0"
#define MDR1_REG "MDR1"
#define CNTR_REG "CNTR"
#define STR_REG "STR"

/************************************************************************************
* Function: initCmdList
* Description: initCmdList stores commands in the cmdList[MAX_CMDS]
*
* Arguments: cmdList - pointer to a structure containing commands
*
* Return: none
*
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: 17.01.2022 (dd.mm.yyyy)
************************************************************************************/
void initCmdList(CMD* cmdList) {

    cmdList[0].cmdName = CMD0; // initialize the first command name; "dutyC"
    cmdList[0].nArgs = CMD0_NARG; // initialize num of arguments in first command
    
    // repeat for all remaining valid commands
    cmdList[1].cmdName = CMD1;      // mtrDir
    cmdList[1].nArgs = CMD1_NARG;

    cmdList[2].cmdName = CMD2;      // brake
    cmdList[2].nArgs = CMD2_NARG;

    cmdList[3].cmdName = CMD3;      // pwmSet
    cmdList[3].nArgs = CMD3_NARG;

    cmdList[4].cmdName = CMD4;      // inputCtrl
    cmdList[4].nArgs = CMD4_NARG;

    cmdList[5].cmdName = CMD5;      // fediHome
    cmdList[5].nArgs = CMD5_NARG;

    cmdList[6].cmdName = CMD6;      // fediClr
    cmdList[6].nArgs = CMD6_NARG;

    cmdList[7].cmdName = CMD7;      // fediRead
    cmdList[7].nArgs = CMD7_NARG;

    cmdList[8].cmdName = CMD8;      // fediDisp
    cmdList[8].nArgs = CMD8_NARG;

    cmdList[9].cmdName = CMD9;      // fediFw
    cmdList[9].nArgs = CMD9_NARG;

    cmdList[10].cmdName = CMD10;    // updateSetPoint
    cmdList[10].nArgs = CMD10_NARG;

}

/************************************************************************************
* Function: parseCmd
*
* Description: parseCmd parses command (cmdLine) received from the console and
*              stores arguments in cmdList->args[MAX_CMDS]
*
* Arguments: cmdList - list of commands
*            cmdLine - pointer to the request (or command) entered;
*
* Return: the index of a command if it is valid; -1 otherwise;
*
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: 17.01.2022 (dd.mm.yyyy)
************************************************************************************/
int parseCmd(CMD * cmdList , char * cmdLine) {
    char* token;
    int valid = -1;
    int counter = 0;
    char *pEnd; // to use only for strtol

    token = strtok(cmdLine, " ");

    // validating command name
    int index = validateCmd(cmdList, token); // stores the index of the command name
    if (index != -1) {
        // validating the number of arguments
        while (token != NULL) {
            token = strtok(NULL, " ");
            if (token == 0x0d) break;   // breaks if '\r' is found
            if (token != NULL && isxdigit(*token) && !isalpha(*token)) {    // for numbers only
                // isxdigit returns > 0 if a hex number; return > 0 if true
                cmdList->args[counter] = atoi(token, &pEnd, 16);   // numArg is a string; convert to an integer
                counter++;
            }
            else if (token != NULL & isalpha(*token)) {   // for registers' names
                cmdList->encRegName = token;
            }
        }
        // varifying if nArgs is equal to the actual number of arguments in the data entered
        if (cmdList[index].nArgs == counter) {
            valid = index;
        }
        else {
            valid = -1;
        }
    }
    else {
        valid = -1;
    }

    cmdLine = 0;   //  clear the rxString for the next entry

    return valid;

}

/************************************************************************************
* Function: validateCmd
*
* Description: validateCmd validates the command entered exists in the cmdList
*
* Arguments: cmdList - list of commands
*            cmdName - pointer to a command entered
*
* Return: index of a command
*
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: 17.01.2022 (dd.mm.yyyy)
************************************************************************************/
int validateCmd(CMD* cmdList, char* cmdName) {

    int i = 0;
    int idx = -1;
    int invalidCmd = 1;

    while (invalidCmd && i < MAX_CMDS) {
        invalidCmd = strcmp(cmdName, cmdList[i++].cmdName);     // returns 0 if true
    }

    if (!invalidCmd) {
        idx = i - 1;
    }

    return idx;
}

/************************************************************************************
* Function: executeCMD
*
* Description: executeCMD uses cmdIndex to execute a command from cmdList->cmdName[cmdIndex].
*
* Arguments: cmdList - list of commands
 *           cmdIndex - integer index to a command from cmdList
*
* Return: 0 if it is valid; -1 otherwise;
*
* Author: Iakov Umrikhin
* Date: 16.01.2022 (dd.mm.yyyy)
* Modified: 17.01.2022 (dd.mm.yyyy)
************************************************************************************/

int executeCmd(CMD * cmdList, int cmdIndex) {
    volatile int status = -1;
    volatile unsigned char fediFwRegStore[4];
    volatile signed int fediFwPosCounts, fediFwAngle, fediFwRev;
    volatile signed long int fediCounts = 0;
    volatile char regBuff[10];
    volatile unsigned char fwRegCntr = 0;
    volatile unsigned char byteRegRead[6];
    volatile signed char dtrArray[4] = {};

    volatile char printStr[50];


    switch(cmdIndex) {
    case 0:     //  dutyC
        status = 0;

        setPWM(cmdList->args[0] * 10, PWM_FRQ);
        sprintf(printStr, "Duty cycle: %d\r\n", cmdList->args[0]);

        // print out the result on the screen
        usciA1UartTxString(printStr);
        newLine();
        newLine();

        break;
    case 1:     // mtrDir
        status = 0;

        if (!cmdList->args[0]) {    // CW mode

            status = vnh7070CW(cmdList->args[1]);
            // valid or invalid command
            if (!status) {
                sprintf(printStr, "CW mode is on\r\n");
            }
            else {
                sprintf(printStr, "Invalid command\r\n");
            }
        }
        else if (cmdList->args[0]) {     // CCW mode

            status = vnh7070CCW(cmdList->args[1]);
            // valid or invalid command
            if (!status) {
                sprintf(printStr, "CCW mode is on\r\n");
            }
            else {
                sprintf(printStr, "Invalid command\r\n");
            }
        }
        else {  // none of the modes
            sprintf(printStr, "Invalid command\r\n");
        }

        // print out the result
        usciA1UartTxString(printStr);
        newLine();
        newLine();

        break;
    case 2:     // brake

        status = vnh7070Brake();

        if (!status) {
            sprintf(printStr, "Brake is set\r\n");
        }
        else {
            sprintf(printStr, "Invalid command\r\n");
        }
        // usciA1UartTxString(printStr);

        break;
    case 3:     // pwmSet

        status = 0;
        // dutyCycle is in percentage: Ex.: 10 == 10%;
        // frequency is in Hz: Ex.: 1 kHz == 1000

        setPWM(cmdList->args[0], cmdList->args[1]);
        break;

    case 4:     // inputCtrl

        status = vnh7070InputCtrl(cmdList->args[0]);
        if (!status) {  // command has been executed
            sprintf(printStr, "Control is set to: 0x%x\r\n", cmdList->args[0]);
        }
        else {
            sprintf(printStr, "Invalid command\r\n");
        }
        // print out the result
        usciA1UartTxString(printStr);
        newLine();
        newLine();

        break; 

    // commands for LS7366R sensor 
    case 5:     // fediHome
        status = 0;

        // checking if a number entered is negative
        if (setPointGlobal < 0) { // negative numbers
            setPointGlobal *= -1;
        }

        // storing LSB
        dtrArray[3] = setPointGlobal;
        dtrArray[2] = setPointGlobal >> 8;
        dtrArray[1] = setPointGlobal >> 16;
        dtrArray[0] = setPointGlobal >> 24;

        angleOffset = setPointGlobal;

        LS7366Rwrite(DTR, dtrArray);    // loading data register with a number of counts
        LS7366Rload(CNTR);      // tranfering counts from DTR to CNTR
        break;
    case 6:     // fediClr
        // should clear the display but the LCD is not connected currently (16.05.2022)
        break;
    case 7:     // fediRead
        memset(fediFwRegStore, 0, 4);   // set fediFwRegStore to 0; 4 byte long
        memset(regBuff, 0, 50);
        if (!strcmp(cmdList->encRegName, MDR0_REG)) { // register MDR0 is to be read
            LS7366Rread(MDR0, byteRegRead);
            // store the first byte in a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            newLine();
        }
        else if (!strcmp(cmdList->encRegName, MDR1_REG)) {  // register MDR1 is to be read
            LS7366Rread(MDR1, byteRegRead);
            // convert a hex number into a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            newLine();
        }
        else if (!strcmp(cmdList->encRegName, STR_REG)) {   // register STR is to be read
            LS7366Rread(STR, byteRegRead);
            // convert a hex number into a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            newLine();
        }
        else if (!strcmp(cmdList->encRegName, CNTR_REG)) {  // register CNTR is to be read
            LS7366Rread(CNTR, fediFwRegStore);
            
            fediCounts = (fediFwRegStore[0] << 24) |
                         (fediFwRegStore[1] << 16) |
                         (fediFwRegStore[2] <<  8) |
                          fediFwRegStore[3];

            sprintf(printStr, "CNTR: %ld \n", fediCounts);

            // go to a new line 
            newLine();

            // printStr from the garbage
            memset(printStr, 0, 50);
        }
        break;
    case 8:     // fediDisp     
        switch(cmdList->args[0]) {  // the first digit after fediDisp
        case 0:     // single line mode
            dispMode = 0;
            break;
        case 1:     // horizontal bar mode
            dispMode = 1;
            break;
        }
        break;
    case 9:    // fediFw
        memset(fediFwRegStore, 0, 4);   // set fediFwRegStore to 0; 4 bytes long
        
        LS7366Rread(CNTR, fediFwRegStore);

        fediCounts = (fediFwRegStore[0] << 24) |
                     (fediFwRegStore[1] << 16) |
                     (fediFwRegStore[2] << 8)  |
                      fediFwRegStore[3];

        // convert from quadrature counts into single counts

        fediFwAngle = (fediCounts - angleOffset) / CNT_TO_ANG;     // compute the angle
        fediFwRev = fediFwAngle / 360;      // compute revolutions

        // need to show only the displacement
        if (fediFwRev >= 0) {    // positive revolutions
            fediFwAngle = fediFwAngle - fediFwRev * ONE_REV;
        }
        else if (fediFwRev < 0) {
            fediFwAngle = fediFwAngle - fediFwRev * ONE_REV;
        }

        sprintf(printStr, "RES: %d   ANGLE:  %d  \r\n", fediFwRev, fediFwAngle);
        newLine();

        break;
    case 10:    // updateSetPoint x y; where x is the sign on the angle; y is the angle

        setPointGlobal = cmdList->args[1] * 13;

        if (cmdList->args[0]) {     // if true, then it is a negative number
            setPointGlobal *= -1;
        }
        
        break;
    }

    return status;  // returns -1 if there is an error; 0 if all good!!

}

/************************************************************************************
* Function: newLine
*
* Description: goes to a new line
*
* Arguments: none
*
* Return: none
*
* Author: Iakov Umrikhin
* Date: 22.03.2022 (dd.mm.yyyy)
* Modified: 22.03.2022 (dd.mm.yyyy)
************************************************************************************/
void newLine(void) {

    usciA1UartTxChar('\r');
    usciA1UartTxChar('\n');

}
