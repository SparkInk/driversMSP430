/*************************************************************************************************
 * ucsiUart.h
 * - - C interface file for MSP430 UCSI UART A1, A0
 *
 *  Author: Greg Scutt
 *  Created on: March 1, 2017
 *  Modified: Feb 19, 2018
 **************************************************************************************************/

#ifndef USCIUART_H_
#define USCIUART_H_

    #define BUF_SIZE 100
    #define MAX_ARGS 4
    #define MAX_CMDS 6
    #define CMD_NARG 3
    #define CMD2_NARG 1
    #define CMD3_NARG 2
    #define CMD4_NARG 0
    #define CMD5_NARG 4
// command name
    #define CMD0 "pDir"
    #define CMD1 "pOut"
    #define CMD2 "p3Out" // assumes P3OUT configured as an output
    #define CMD3 "nokLcdDrawScrnLine"
    #define CMD4 "nokLcdClear"
    #define CMD5 "nokLcdDrawLine"
// global variables
    char rxBuffer[BUF_SIZE];

    typedef struct CMD {
        const char* cmdName;
        int nArgs; // number of input arguments for a command
        int args[MAX_ARGS]; // arguments
    }CMD;

// functions
    void usciA1UartInit();

    void usciA1UartTxChar(unsigned char txChar);

    void usciA1UartTxString(unsigned char* txChar);

    char *usciA1UartGets(char *rxString);

    int parseCmd(CMD * cmdList , char * cmdLine);

    int validateCmd(CMD * cmdList ,char * cmdName);

    int executeCmd(CMD * cmdList, int cmdIndex);

    void initCmdList(CMD* cmdList);
#endif /* USCIUART_H_ */
