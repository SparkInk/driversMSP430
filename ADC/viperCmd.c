/*****************************************************************
 * This module is a command parser for the VNH7070AS
 *
 *
 * Author: Iakov Umrikhin aka Mr. Smile
 *
 * Date Created: 21.03.2022
 * Date Modified: 21.03.2022
 *****************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <viperCmd.h>
#include "vnh7070API.h"
#include "usciUart.h"
#include "timerA0.h"

// global variables
CMD cmds[MAX_CMDS]; //this is an array cmds of type CMD
ENC_STATE encState = {0, 0};

volatile unsigned int countDiff;
unsigned char enterConsole;
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

//     dutyC
    cmdList[0].cmdName = CMD0; // initialize the first command name; pDir
    cmdList[0].nArgs = CMD0_NARG; // initialize num of arguments in first command

//    mtrDir
    cmdList[1].cmdName = CMD1;
    cmdList[1].nArgs = CMD1_NARG;

//    brake
    cmdList[2].cmdName = CMD2;
    cmdList[2].nArgs = CMD2_NARG;

//    dispSpeed
    cmdList[3].cmdName = CMD3;
    cmdList[3].nArgs = CMD3_NARG;

//    pwmSet
    cmdList[4].cmdName = CMD4;
    cmdList[4].nArgs = CMD4_NARG;

//    inputCtrl
    cmdList[5].cmdName = CMD5;
    cmdList[5].nArgs = CMD5_NARG;

//    dispSpeed
    cmdList[6].cmdName = CMD6;
    cmdList[6].nArgs = CMD6_NARG;

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
* Modified: 21.03.2022 (dd.mm.yyyy)
************************************************************************************/

int executeCmd(CMD * cmdList, int cmdIndex) {
    volatile int status = -1;

    volatile char printStr[20];
    volatile char fwVelocityStr[15] = "FW Velocity: \n";
    volatile char rpmStr[6] = " rpm\n";
    volatile char cwStr[7] = " CW \n";
    volatile char ccwStr[7] = " CCW \n";
    volatile unsigned int speed;

    switch(cmdIndex) {
    // commands for Nokia 5110 display
    case 0:     // dutyC
        status = 0;

        setPWM(cmdList->args[0] * 10, PWM_FRQ);
        sprintf(printStr, "Duty cycle: %d\r\n", cmdList->args[0]);

        // print out the result on the screen
        usciA1UartTxString(printStr);
        newLine();
        newLine();

        break;
    case 1:     // mtrDir; CW -> 0; CCW -> 1; divide dutyCyc
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

        usciA1UartTxString(printStr);
        newLine();
        newLine();

        break;
    case 3:     // dispSpeed
        status = 0;

        while (enterConsole != '\r') {
            UCA1IE |= UCRXIE;
            if (displayRPM) {

                speed = (3255) / (countDiff);
                displayRPM = 0;
                usciA1UartTxChar('\r');
//              usciA1UartTxChar('\n');
                usciA1UartTxString(fwVelocityStr);
                dispCnsl(speed);
                usciA1UartTxString(rpmStr);

                // display the direction
                if (fwDirection(encState)) {  // cw
                    usciA1UartTxString(cwStr);
                }
                else {  // ccw
                    usciA1UartTxString(ccwStr);

                }
            }
        }
//      clear enterConsole
       enterConsole = 0;

        break;
    case 4:     //  pwmSet

        status = 0;
        // dutyCycle is in percentage: Ex.: 10 == 10%;
        // frequency is in Hz: Ex.: 1 kHz == 1000

        setPWM(cmdList->args[0], cmdList->args[1]);
        break;
    case 5:     // inputCtrl

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
///********************************************
// *              TimerA0 ISR                 *
// *                                          *
// * Updates the speed at 60 Hz               *
// *******************************************/
//
//#pragma vector = TIMER1_A0_VECTOR
//__interrupt void timerA0ISR(void) {
//
//
//    displayRPM = 1;
//
//    // clear interrupt flags
//    TA1CTL &= ~TAIFG;
//}
///************************************************
// *              TimerA1 ISR                     *
// *                                              *
// * Used to compute the speed of the flywheel    *
// * with the capture mode                        *
// ***********************************************/
//
//#pragma vector = TIMER0_A1_VECTOR
//__interrupt void timerA1ISR(void) {
//
//    static volatile unsigned int oldCounts = 0;
//    volatile unsigned int newCounts;
//
//    //disable interrupt
//    TA0CCTL1 &= ~CCIE;
//
//    // record values of CHA and CHB
//    encState.chAState = 1;
//    encState.chBState = (P1IN & 0xFF) >> 3;     // shift the P1.3 bit by 3
//
//    newCounts = TA0R;
//
//    // record the current counts on TA0R
//    countDiff = newCounts- oldCounts;
//
//    oldCounts = newCounts;
//
//    // clear interrupt flag; in case, the vector did not work
//    TA0CCTL1 &= ~CCIFG;
//
//    // enable interrupt
//    TA0CCTL1 |= CCIE;
//}

/********************************
 *      UART Interrupt          *
 *                              *
 * Used to trigger on ENTER key *
 ********************************/

#pragma vector = USCI_A1_VECTOR
 __interrupt void enterConsISR (void) {

     // disable interrupts
     UCA1IE &= ~UCRXIE;
     enterConsole = 0x0d;   // new line
     UCA1IFG &= ~UCRXIFG;

 }

