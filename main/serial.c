/******************************************************
 * Original author: Youngchang Yoo (frog@falinux.com) *
 * Modified by Yeonwoo Sung on 2018. 4. 29..          *
 * Most rountines of this could are from blob.        *
 ******************************************************/

#include <pxa255.h>
#include <config.h>
#include <time.h>
#include <stdio.h>
#include <gpio.h>

//Changes the __REG macro in the pxa255.h file to the normal value.
#define __REG(x) (x)


// Definitions for the global variables.
static int SerialErrorFlag = 0; //The error flag of the serial device.

static volatile Word *UART = (volatile Word *) STUART; // base address of the UART

#define UART_DATA          	((volatile Word) UART[0] )  // data of the UART
#define UART_IER		((volatile Word) UART[1] )  // permission for the interrupt
#define UART_FCR		((volatile Word) UART[2] )  // the interrupt status
#define UART_LCR		((volatile Word) UART[3] )  // the line control
#define UART_MCR		((volatile Word) UART[4] )  // the modem control
#define UART_LSR		((volatile Word) UART[5] )  // the line status
#define UART_MSR		((volatile Word) UART[6] )  // the modem status
#define UART_SPR		((volatile Word) UART[7] )
#define UART_ISR		((volatile Word) UART[8] )  // select the infrared port

#define UART_DLL		((volatile Word) UART[0] )  // UART line status (low)
#define UART_DLH           	((volatile Word) UART[1] )  // UART line status (high)

//Send the single character to the serial device.
void SerialOutChar( const char c  ) {
    //Waits until the character could be sent.
    while(( UART_LSR & LSR_TDRQ ) == 0 ) ;

    //print out the data to the serial.
    UART_DATA = c;
}

//Sends the single character to the serial device.
//This function converts the LR of the CR to CR when it prints out the character.
void SerialOutChar_CheckCR( const char c  ) {
    SerialOutChar( c );
    if(c == '\n') SerialOutChar('\r');
}

//Sends the string (character stream) to the serial device.
int SerialOutStr( char *str, int size ) {
    int	lp ;

    for (lp=0;lp<size;lp++) SerialOutChar(str[lp]) ;

    return lp;
}

//Sends the string (character stream) to the serial device.
//This function converts the LR of the CR to CR when it prints out the string.
int SerialOutStr_CheckCR( char *str, int size ) {
    int	lp ;

    for (lp=0;lp<size;lp++) SerialOutChar_CheckCR(str[lp]) ;

    return lp;
}

//Check if the serial device sent any data.
//If so, returns 1. Otherwise, returns 0.
int SerialIsReadyChar( void ) {
    //Check if there is any received data
    if( UART_LSR & LSR_DR ) return 1;
    return 0;
}

//Returns the serial status.
//If there is some serialized data, returns 1.
//Otherwise, returns 0.
int SeriallGet_LSR( void )
{
    return UART_LSR;
}

//Reads the error status from the serial device.
//Then returns the received character.
char SerialIsGetChar( void ) {
    //Gets the error codes.
    SerialErrorFlag = UART_LSR & (LSR_PE | LSR_FE | LSR_OE);

    //returns the received data
    return (char) UART_DATA;
}

//Clears the error flag of the serial device.
char SerialIsClearError( void ) {
    SerialErrorFlag = 0;
    return (char)SerialErrorFlag;
}

//Gets the error flag value.
int SerialIsGetError( void ) {
    return SerialErrorFlag;
}
