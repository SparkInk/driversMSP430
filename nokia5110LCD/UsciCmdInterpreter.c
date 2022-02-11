/*
 * UsciCmdInterpreter.c
 *
 *  Created on: 16 Jan 2022
 *      Author: emine
 */
#include <msp430.h>
#include <string.h>
#include "usciUart.h"
#include "nok5110LCD.h"

CMD cmds[MAX_CMDS]; //this is an array cmds of type CMD

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
    cmdList[0].nArgs = CMD_NARG; // initialize num of arguments in first command
    // repeat for all remaining valid commands
    cmdList[1].cmdName = CMD1;  // pOut
    cmdList[1].nArgs = CMD_NARG;

    cmdList[2].cmdName = CMD2;  // p3Out
    cmdList[2].nArgs = CMD2_NARG;

    cmdList[3].cmdName = CMD3;  // nokLcdDrawScrnLine
    cmdList[3].nArgs = CMD3_NARG;

    cmdList[4].cmdName = CMD4;  // nokLcdClear

    cmdList[4].nArgs = CMD4_NARG;

    cmdList[5].cmdName = CMD5;  // nokLcdDrawLine
    cmdList[5].nArgs = CMD5_NARG;
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
            if (token == 0x0d) break;
            if (token != NULL && isxdigit(*token)) {
                // isxdigit returns > 0 if a hex number; return > 0 if true
                cmdList->args[counter] = strtol(token, &pEnd, 16);   // numArg is a string; convert to an integer
                counter++;
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
    switch(cmdIndex) {
    case 0:     // "pDir"
        status = 0;
        switch(cmdList->args[0]) { // going through commands' indexes
        case 1:     // P1DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P1DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P1DIR |= cmdList->args[1];
            }
            break;
        case 2:     // P2DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P2DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P2DIR |= cmdList->args[1];
            }
            break;
        case 3:     // P3DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P3DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P3DIR |= cmdList->args[1];
            }
            break;
        case 4:     // P4DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P4DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P4DIR |= cmdList->args[1];
            }
            break;
        case 5:     // P5DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P5DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P5DIR |= cmdList->args[1];
            }
            break;
        case 6:     // P6DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P6DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P6DIR |= cmdList->args[1];
            }
            break;
        case 7:     // P7DIR
            status = 0;
            if (cmdList->args[2] == 0) {    // PxDIR = 0 -> clear mask's bits
                P7DIR &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxDIR = 1 -> set mask's bits
                P7DIR |= cmdList->args[1];
            }
            break;
        }
        break;
    case 1:     // "pOut"
        switch(cmdList->args[0]) {
        case 1:     // P1OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P1OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P1OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P1OUT ^= cmdList->args[1];
            }
            break;
        case 2:     // P2OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P2OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P2OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P2OUT ^= cmdList->args[1];
            }
            break;
        case 3:     // P3OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P3OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P3OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P3OUT ^= cmdList->args[1];
            }
            break;
        case 4:     // P4OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P4OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P4OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P4OUT ^= cmdList->args[1];
            }
            break;
        case 5:     // P5OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P5OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P5OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P5OUT ^= cmdList->args[1];
            }
            break;
        case 6:     // P6OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P6OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P6OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P6OUT ^= cmdList->args[1];
            }
            break;
        case 7:     // P7OUT
            status = 0;
            if (cmdList->args[2] == 0) {    // PxOUT = 0 -> clear mask's bits
                P7OUT &= ~cmdList->args[1];
            }
            if (cmdList->args[2] == 1) {    // PxOUT = 1 -> set mask's bits
                P7OUT |= cmdList->args[1];
            }
            if (cmdList->args[2] == 2) {    // PxOUT = 2 -> toggles mask's bits
                P7OUT ^= cmdList->args[1];
            }
            break;
        }
        break;
    case 2: // p3Out
        status = 0;
        P3OUT |= cmdList->args[0];   // assigns a mask to P3OUT
        break;
    case 3:     //  nokLcdDrawScrnLine
        status = 0;
        nokLcdDrawScrnLine(cmdList->args[0], cmdList->args[1]);
        break;
    case 4:     //  nokLcdClear
        status = 0;
        nokLcdClear();
        break;
    case 5:     // nokLcdDrawLine
        status = 0;
        nokLcdDrawLine(cmdList->args[0], cmdList->args[1], cmdList->args[2], cmdList->args[3]);
        break;
    }

    return status;  // returns -1 if there is an error; 0 if all good!!

}
