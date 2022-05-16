/**************************************************************
 * This is a header file for the motor control module
 *
 * Author: Iakov Umrikhin
 *
 * Date Created: 22.03.2022
 * Date Modified: 22.03.2022
 **************************************************************/

#ifndef VIPERCMD_H_
#define VIPERCMD_H_

    #define MAX_ARGS 2
    #define MAX_CMDS 7
    #define CMD0_NARG 1
    #define CMD1_NARG 2
    #define CMD2_NARG 0
    #define CMD3_NARG 0
    #define CMD4_NARG 2
    #define CMD5_NARG 1
    #define CMD6_NARG 0

    // command name
    #define CMD0 "dutyC"
    #define CMD1 "mtrDir"   // implements both mtrCW and mtrCCW
    #define CMD2 "brake"
    #define CMD3 "dispSpeed"
    #define CMD4 "pwmSet"
    #define CMD5 "inputCtrl"
    #define CMD6 "dispSpeed"
    // macros

    // global variables
    typedef struct CMD {
        const char* cmdName;
        int nArgs; // number of input arguments for a command
        unsigned int args[MAX_ARGS]; // arguments
        char *encRegName;
    }CMD;

    // functions' definitions
    void newLine(void);

#endif /* VIPERCMD_H_ */
