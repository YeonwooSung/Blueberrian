/**********************************************************************************
 * The aim of this file is to implement the timer functions for the Blueberrian.  *
 *                                                                                *
 * author: Yeonwoo Sung                                                           *
 **********************************************************************************/

#include <pxa255.h>
#include <time.h>

static int numOverflows; //to count the number of timer overflow issues

/* The setter for the watchdog. Gets the miliseconds as a parameter. */
void SetWatchdog (int msec) {
    OWER = OWER_WME;
    OSSR = OSSR_M3;
    OSMR3 = OSCR + (36864*(msec/10));
}

/* The timer initialiser. */
void TimerInit(void) {
    // Initialise the timer value.
    OSCR = 0;

    // Bans the every interrupt that are occurred by the timer.
    OIER = 0;

    // Waits until the timer counter value goes 0.
    while(OSCR == 0); // WatchDoc() ;

    // Set the match value to 0 by setting the match register as 0.
    OSMR0 = 0;

    // Initialise the match status of the match register.
    OSSR = OSSR_M0;

    // Initialise the numOverflows
    numOverflows = 0;
}

/* Get the value of the timer. */
unsigned int  TimerGetTime(void) {
    return ((unsigned int) OSCR);
}

/* Detect the timer overflow. */
int TimerDetectOverflow(void) {
    return(OSSR & OSSR_M0);
}

/* Clear the overflow status of the timer. */
void TimerClearOverflow(void) {
    if( TimerDetectOverflow() ) numOverflows++;

    OSSR = OSSR_M0;
}

/*
 * Delays for miliseconds.
 * (The maximum miliseconds that this function could handle is 1160 seconds)
 */
void msleep(unsigned int msec) {
    ReloadTimer( 0, msec );
    while( 0 == TimeOverflow(0) );
    FreeTimer( 0 );
}

/* Reloads the timer. */
void ReloadTimer( unsigned char bTimer, unsigned int msec ) {
    unsigned long reg;

    bTimer &= 0x03;
    reg = (1 << bTimer);

    // Clears the timer overflow bit.
    OSSR  = reg;

    // detects when the overflow would be occurred.
    switch (bTimer) {
        case 0 : OSMR0 = OSCR + (TICKS_PER_SECOND/1000)*msec; break;
        case 1 : OSMR1 = OSCR + (TICKS_PER_SECOND/1000)*msec; break;
        case 2 : OSMR2 = OSCR + (TICKS_PER_SECOND/1000)*msec; break;
        case 3 : OSMR3 = OSCR + (TICKS_PER_SECOND/1000)*msec; break;
    }

    // activate the timer interrupt.
    OIER = OIER | reg;
}

// Activate the timer overflow.
int TimeOverflow( unsigned char bTimer ) {
    unsigned long reg;

    reg = 1 << (bTimer&0x03);
    return (OSSR&reg);
}

// Free the timer by deactivate all timer interrupts.
void    FreeTimer( unsigned char bTimer )
{
    unsigned long mask;

    mask = 1 << (bTimer&0x03);
    OIER = OIER&(~mask); // deactivate all timer interrupts.
}
