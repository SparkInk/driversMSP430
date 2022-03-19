/*
 * UsciCmdInterpreter.c
 *
 *  Created on: 16 Jan 2022
 *      Author: emine
 */
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include "usciUart.h"
#include "nok5110LCD.h"
#include "angleDisp.h"
#include "LS7366R.h"

// global variables
CMD cmds[MAX_CMDS]; //this is an array cmds of type CMD

unsigned char dispMode;     // display mode: 0 -> single line mode; 1 -> horizontal bar mode
volatile signed int angleOffset;

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

    cmdList[0].cmdName = CMD0; // initialize the first command name; pDir
    cmdList[0].nArgs = CMD0_NARG; // initialize num of arguments in first command
    // repeat for all remaining valid commands
    cmdList[1].cmdName = CMD1;  // pOut
    cmdList[1].nArgs = CMD1_NARG;

    cmdList[2].cmdName = CMD2;  // p3Out
    cmdList[2].nArgs = CMD2_NARG;

    cmdList[3].cmdName = CMD3;  // nokLcdDrawScrnLine
    cmdList[3].nArgs = CMD3_NARG;

    cmdList[4].cmdName = CMD4;  // nokLcdClear

    cmdList[4].nArgs = CMD4_NARG;

    cmdList[5].cmdName = CMD5;  // nokLcdDrawLine
    cmdList[5].nArgs = CMD5_NARG;

    cmdList[6].cmdName = CMD6;  // fediHome
    cmdList[6].nArgs = CMD6_NARG;

    cmdList[7].cmdName = CMD7;  // fediClr
    cmdList[7].nArgs = CMD7_NARG;

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
                cmdList->args[counter] = strtol(token, &pEnd, 10);   // numArg is a string; convert to an integer
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

    switch(cmdIndex) {
    // commands for Nokia 5110 display
    case 0:     //  nokLcdDrawScrnLine
        status = 0;
        nokLcdDrawScrnLine(cmdList->args[0], cmdList->args[1]);
        break;
    case 1:     //  nokLcdClear
        status = 0;
        nokLcdClear();
        break;
    case 2:     // nokLcdDrawLine
        status = 0;
        nokLcdDrawLine(cmdList->args[0], cmdList->args[1], cmdList->args[2], cmdList->args[3]);
        break;
    // commmand for LS7366R sensor 
    case 3:     // fediHome
        status = 0;

        // checking if a number entered is negative
        if (cmdList->args[1]) { // negative numbers
            cmdList->args[0] *= -1;
        }

        // storing LSB
        dtrArray[3] = cmdList->args[0];
        dtrArray[2] = cmdList->args[0] >> 8;
        dtrArray[1] = cmdList->args[0] >> 16;
        dtrArray[0] = cmdList->args[0] >> 24;

        angleOffset = cmdList->args[0];

        LS7366Rwrite(DTR, dtrArray);    // loading data register with a number of counts
        LS7366Rload(CNTR);      // tranfering counts from DTR to CNTR
        nokLcdClear();
        break;
    case 4:     // fediClr
        nokLcdClear();
        break;
    case 5:     // fediRead
        memset(fediFwRegStore, 0, 4);   // set fediFwRegStore to 0; 4 byte long
        memset(regBuff, 0, 50);
        if (!strcmp(cmdList->encRegName, MDR0_REG)) { // register MDR0 is to be read
            LS7366Rread(MDR0, byteRegRead);
            // store the first byte in a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            usciA1UartTxChar('\r');
            usciA1UartTxChar('\n');
        }
        else if (!strcmp(cmdList->encRegName, MDR1_REG)) {  // register MDR1 is to be read
            LS7366Rread(MDR1, byteRegRead);
            // convert a hex number into a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            usciA1UartTxChar('\r');
            usciA1UartTxChar('\n');
        }
        else if (!strcmp(cmdList->encRegName, STR_REG)) {   // register STR is to be read
            LS7366Rread(STR, byteRegRead);
            // convert a hex number into a string
            sprintf(byteRegRead, "0x%x\n", byteRegRead[0]);
            usciA1UartTxString(byteRegRead);

            // go to a new line
            usciA1UartTxChar('\r');
            usciA1UartTxChar('\n');
        }
        else if (!strcmp(cmdList->encRegName, CNTR_REG)) {  // register CNTR is to be read
            LS7366Rread(CNTR, fediFwRegStore);
            
            fediCounts = (fediFwRegStore[0] << 24) |
                         (fediFwRegStore[1] << 16) |
                         (fediFwRegStore[2] << 8)  |
                         fediFwRegStore[3];

            displayCounts(fediCounts);

            // go to a new line 
            usciA1UartTxChar('\r');
            usciA1UartTxChar('\n');
        }
        break;
    case 6:     // fediDisp     
        switch(cmdList->args[0]) {  // the first digit after fediDisp
        case 0:     // single line mode
            dispMode = 0;
            break;
        case 1:     // horizontal bar mode
            dispMode = 1;
            break;
        }
        break;
    case 7:    // fediFw
        memset(fediFwRegStore, 0, 4);   // set fediFwRegStore to 0; 4 bytes long
        char revStr[5] = "REV:\n";
        char angleStr[7] = "ANGLE:\n";
        
        LS7366Rread(CNTR, fediFwRegStore);

        fediCounts = (fediFwRegStore[0] << 24) |
                     (fediFwRegStore[1] << 16) |
                     (fediFwRegStore[2] << 8)  |
                      fediFwRegStore[3];

        // convert from quadrature counts into single counts

        fediFwAngle = (fediCounts - angleOffset)/ CNT_TO_ANG;     // compute the angle
        fediFwRev = fediFwAngle / 360;      // compute revolutions

        // need to show only the displacement
        if (fediFwRev >= 0) {    // positive revolutions
            fediFwAngle = fediFwAngle - fediFwRev * ONE_REV;
        }
        else if (fediFwRev < 0) {
            fediFwAngle = fediFwAngle - fediFwRev * ONE_REV;
        }
//        usciA1UartTxChar('\n');
        // diplay revolutions
        usciA1UartTxString(revStr);
        displayCounts(fediFwRev);
        // display angle
        usciA1UartTxString(angleStr);
        displayCounts(fediFwAngle);

        usciA1UartTxChar('\r');
        usciA1UartTxChar('\n');

        break;
    }

    return status;  // returns -1 if there is an error; 0 if all good!!

}
