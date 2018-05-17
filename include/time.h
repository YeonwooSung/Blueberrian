/**********************************************************************
 * The header file for the time stuff that will be used in the ezboot.*
 * Author: Yeonwoo Sung                                               *
 **********************************************************************/

#ifndef BLUEBERRIAN_TIME_H
#define BLUEBERRIAN_TIME_H

#define TICKS_PER_SECOND 3686400

void		SetWatchdog( int msec );
void  		TimerInit(void); // Initialse the timer.
unsigned int  	TimerGetTime(void); // return the value of ( 1/TICKS_PER_SECOND )
int   		TimerDetectOverflow(void);
void  		TimerClearOverflow(void);
void  		msleep(unsigned int msec); // sleeps for miliseconds

extern void 	ReloadTimer( unsigned char bTimer, unsigned int msec);
extern int  	TimeOverflow( unsigned char bTimer );
extern void 	FreeTimer( unsigned char bTimer );

#endif //BLUEBERRIAN_TIME_H
