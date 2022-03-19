
/*
 * angleDisp.h
 *
 *  Created on: Feb. 11, 2022
 *      Author: spark-inc aka Iakov Umrikhin
 */

#ifndef ANGLEDISP_H_
#define ANGLEDISP_H_

// macros for constant variables
    #define CNT_TO_ANG  13  // 13 pulses == 1 degree of the flywheel
    #define RADIUS  24      // radius of the line on the screen
    #define PI  3.1415926535
    #define RADTODEG  PI / 180  // conversion factor from radians to degrees
    #define ONE_REV 360
    #define UPDATE_RATE_BAR 54
    #define UPDATE_RATE_LINE 26    
// global variables
    unsigned char dispMode;     // display mode: 0 -> single line mode; 1 -> horizontal bar mode

// functions' prototypes
    /************************************************************************************
    * Function: displayCounts
    * Description: displays posCounts on a console
    *
    * Arguments: posCounts  -   a number of counts from the encoder LS7366R
    *
    * return: none
    * Author: Iakov Umrikhin
    * Date: 06.02.2022 (dd:mm:yyyy)
    * Modified: 11.02.2022 (dd:mm:yyyy)
    ************************************************************************************/
    void displayCounts(signed long int posCounts);

    /************************************************************************************
     * Function: dispAngleConsole
     *
     * Description: this function displays an angle (positive or negative) on the console
     *              instead of the counts, because the life is complicated enough
     *
     * Arguments:   posCounts   -   a number of counts from the encoder LS7366R
     *
     * Return:  none
     *
     * Author: Iakov Umrikhin
     *
     * Date Created: 11.02.2022 (dd:mm:yyyy)
     * Date Modified: 14.02.2022 (dd:mm:yyyy)
     ************************************************************************************/
    void dispAngleConsole (signed long int posCounts);

    signed int dispAngleLCDline (signed long int posCounts);

    signed int dispAngleLCDbar (signed long int posCounts);

    /************************************************************************************
     * Function: cwRotation(angle, newXi, newYi, newXf, newYf)
     *
     * Description: a shortcut for a clockwise rotation
     *
     * Arguments:  angle - positive angle in degrees
     *             newXi, newYi - initial coordinates for the line to be drawn
     *             newXf, newYf - final coordinates for the line to be drawn
     *
     * Return:  this function doesn't owe you anything
     *
     * Author: Iakov Umrikhin
     *
     * Date Created: 11.02.2022 (dd:mm:yyyy)
     * Date Modified: 14.02.2022 (dd:mm:yyyy)
     ************************************************************************************/
    void cwRotation(signed int angle, unsigned int newXi, unsigned int newYi, unsigned int newXf, unsigned int newYf);

    /************************************************************************************
     * Function: ccwRotation(angle, newXi, newYi, newXf, newYf)
     *
     * Description: a shortcut for a counter-clockwise rotation
     *
     * Arguments:  angle - positive angle in degrees
     *             newXi, newYi - initial coordinates for the line to be drawn
     *             newXf, newYf - final coordinates for the line to be drawn
     * 
     * Return:  none
     *
     * Author: Iakov Umrikhin
     *
     * Date Created: 11.02.2022 (dd:mm:yyyy)
     * Date Modified: 14.02.2022 (dd:mm:yyyy)
     ************************************************************************************/
    void ccwRotation(signed int angle, unsigned int newXi, unsigned int newYi, unsigned int newXf, unsigned int newYf);

#endif /* ANGLEDISP_H_ */
