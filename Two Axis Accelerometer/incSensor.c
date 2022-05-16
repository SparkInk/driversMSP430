/*********************************************
 * incSensor.c
 *
 * Author: Iakov Umrikhin
 * 
 * Date Created: 02.05.2022 (dd::mm::yyyy)
 * Date Modfied: 02.05.2022 (dd::mm::yyyy)
 *********************************************/

#include <msp430.h>
#include <math.h>
#include "adc12.h"
#include "incSensor.h"

/***********************************************************************
 * Name: windowAvg
 * 
 * Description: windowAvg averages 5 elements in the array 
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 02.05.2022 (dd::mm::yyyy)
 * Date Modified: 02.05.2022 (dd::mm::yyyy)
 ***********************************************************************/
signed int windowAvg(unsigned int *array){

    volatile unsigned char i;
    volatile signed int average;

    // sum all elements of the array
    for (i = 1; i < ARRAY_SIZE; i++) {
        average += *(array + i);
    }

    // compute the arithmetic average
    average  = average >> 3;

    return average;
}
/***********************************************************************
 * Name: angleAvgCompute
 * 
 * Description: 
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created: 02.05.2022 (dd::mm::yyyy)
 * Date Modified: 02.05.2022 (dd::mm::yyyy)
 ***********************************************************************/

signed int angleRealCompute(unsigned int *arrayX, unsigned int *arrayY){

    volatile signed int realAngle;
    volatile signed int xReal, yReal;

    // compute the real angle; use first values from the arrays
    xReal = *(arrayX + 0) - 2048;
    yReal = *(arrayY + 0) - 2048;

    realAngle = atan2(xReal, yReal) * 57.3;

    return realAngle;
}

/***********************************************************************
 * Name: angleAvgCompute
 * 
 * Description: 
 * 
 * 
 * Author: Iakov Umrikhin
 * 
 * Date Created:02.05.2022 (dd::mm::yyyy)
 * Date Modified:02.05.2022 (dd::mm::yyyy)
 ***********************************************************************/

signed int angleAvgCompute(unsigned int *arrayX, unsigned int *arrayY){

    volatile signed int xAvg;
	volatile signed int yAvg;
    volatile signed int averageAngle;

    // windowAvg(array1, array2)
    xAvg = windowAvg(arrayX);
    yAvg = windowAvg(arrayY);

    // convert the range from (0, 4096) to (-2048, 2048)
    xAvg -= 2048;
    yAvg -= 2048;

    // compute the average angle
    averageAngle = atan2(xAvg, yAvg) * 57.3;

    return averageAngle;
}
