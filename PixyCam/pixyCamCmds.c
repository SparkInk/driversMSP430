/*
 * UsciCmdInterpreter.c
 *
 *  Created on: 16 Jan 2022
 *      Author: emine
 */
#include <msp430.h>
#include <string.h>
#include "usciUart.h"
#include "pixyCam2.h"
#include "pixyCamCmds.h"

unsigned char enterConsole;
unsigned char timerUpdate;
void initCmdList(PIXY_CMD* cmdList) {

    cmdList[0].cmdName = CMD0_PIXY; // initialize the first command name; pixyVer
    cmdList[0].nArgs = CMD0_PIXY_NARG; // initialize num of arguments in first command

    // repeat for all remaining valid commands
    cmdList[1].cmdName = CMD1_PIXY;  // pixyVec
    cmdList[1].nArgs = CMD1_PIXY_NARG;

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
int parseCmd(PIXY_CMD * cmdList , char * cmdLine) {
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
int validateCmd(PIXY_CMD* cmdList, char* cmdName) {

    int i = 0;
    int idx = -1;
    int invalidCmd = 1;

    while (invalidCmd && i < MAX_PIXY_CMDS) {
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

 int executeCmd(PIXY_CMD * cmdList, int cmdIndex) {
     volatile int status = -1;
     unsigned char verStr[GET_VERSION_VER_NUM_BYTES];
     unsigned char verStrCounter = 0;
     unsigned char vecStr[VECTOR_NUM_BYTES];
     unsigned char vecStrCounter = 0;

     unsigned coordCounter = 0;
     long long int version = 0;
     char versionCounter = 0;

     double angle;
     char versionStr[10] = "Version:\n";
     char revStr[5] = "Coordinates:\n";
     char x0CoordStr[5] = "X0:\n";
     char x1CoordStr[5] = "X1:\n";
     char y0CoordStr[5] = "Y0:\n";
     char y1CoordStr[5] = "Y1:\n";
     char angleStr[7] = "ANGLE:\n";



     switch(cmdIndex) {
     case 0:    // pixyVer
        pixyGetVersion(PIXY_ADDR, verStr);
        // convert array into an long int
        for (versionCounter; versionCounter < 7; versionCounter++){
            version += verStr[versionCounter] << (versionCounter * 8);
        }
        usciA1UartTxString(versionStr);
        dispVersion(version);
//        for
        break;
     case 1:    // pixyVec

         while(enterConsole != '\r') {
             if (timerUpdate && cmdList->args[0] && !cmdList->args[1]) { // continuous mode is on
                 // enable UART interrupts
                 UCA1IE |= UCRXIE;
                 pixyGetVector(vecStr);
                 angle = computePixyAngle(vecStr[0], vecStr[1], vecStr[2], vecStr[3]);
                 angle = (int)angle;

                 // display angle
                 usciA1UartTxString(angleStr);
                 dispCnsl(angle);
                 timerUpdate = 0;
             }
             if (timerUpdate && cmdList->args[1] && cmdList->args[0]) { // display coordinates mode is on
                  pixyGetVector(vecStr);
                  angle = computePixyAngle(vecStr[0], vecStr[1], vecStr[2], vecStr[3]);
                  angle = (int)angle;

                  // display coordinates
                  //for (coordCounter; coordCounter < 4; coordCounter++){

                  // disp x0
                  usciA1UartTxString(x0CoordStr);
                  dispCoord(vecStr[0]);

                  // disp y0
                  usciA1UartTxString(y0CoordStr);
                  dispCoord(vecStr[1]);

                  // disp x1
                  usciA1UartTxString(x1CoordStr);
                  dispCoord(vecStr[2]);

                  // disp y1
                  usciA1UartTxString(y1CoordStr);
                  dispCoord(vecStr[3]);

                  // display angle
                  usciA1UartTxString(angleStr);
                  dispCnsl(angle);
                  //}
                  timerUpdate = 0;

              }
         }
         enterConsole = 0;
         break;

     } 

     return status;  // returns -1 if there is an error; 0 if all good!!

 }

#pragma vector = USCI_A1_VECTOR
 __interrupt void enterConsISR (void) {

     // disable interrupts
     UCA1IE &= ~UCRXIE;
     enterConsole = 0x0d;   // new line
     UCA1IFG &= ~UCRXIFG;

 }

 /***************************************************
  * ISR for the CCR0 of TimerA0
  * ************************************************/
 #pragma vector = TIMER0_A0_VECTOR
 __interrupt void timer0A0ISR (void) {

     // update LCD
     timerUpdate = 1;

     //TA0CTL &= ~TAIFG;   //automatically clears interrupt flags;
 }
/*******************************************************

Pseudo code:

1. Store the command in a buffer (received from a console)
2. Execute from executeCmd() 
	2.1 pixyVec: displays coordinates of a vector at frame-rate
		     of the PixyCam (60 FPS == 60 Hz) by means of a timer;
		     Has two parameters: cont and disp
		     cont: 1 - display coordinates continuously; 0 - just once;
		     disp: 1 - 

case 1: // pixyVec cmd is chosen
    unsigend char vecStr[4];    // vector coordinates(hex)

	while(enterConsole != '\r' && cont) {    // until user presses Enter && cont = 1
        pixyGetVector(vecStr);
        //display coordinates on console (decimal);
        dispCoord(vecStr);
        // calcualate and display the orientation of the line
        pixyGetOrient(vecStr);

	}   
    // cont == 0 ~> diplay coordinates once
    pixyGetVector(vecStr);
    dispCoord(vecStr);
    
*******************************************************/
