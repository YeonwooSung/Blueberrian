/***************************************************************
 * The header file for the serial.c file of the Blueberrian.   *
 * Author: Yeonwoo Sung                                        *
 ***************************************************************/

#ifndef BLUEBERRIAN_SERIAL_H
#define BLUEBERRIAN_SERIAL_H

// The value that corresponds to each baud rate when using the internal clock of the PXA250.

typedef enum {
    BAUD_4800    = 192 ,
    BAUD_9600    =  96 ,
    BAUD_19200   =  48 ,
    BAUD_38400   =  24 ,
    BAUD_57600   =  16 ,
    BAUD_115200  =   8 ,
    BAUD_230400  =   4 ,
    BAUD_307200  =   3 ,
    BAUD_460800  =   2 ,
    BAUD_921600  =   1
} eBauds;

extern void SerialInit( eBauds baudrate);		// 메인 시리얼 초기화
extern void SerialOutChar( const char c  );    	// 시리얼에 한 문자를 출력한다.
extern int  SerialOutStr( char *str, int size );	// 시리얼에 버퍼의 내용을 출력한다.
extern void SerialOutChar_CheckCR( const char c  );    // 시리얼에 한 문자를 출력한다.
extern int  SerialOutStr_CheckCR( char *str, int size ); // 시리얼에 버퍼의 내용을 출력한다.

extern int SerialIsReadyChar( void );
extern char SerialIsGetChar( void );
extern char SerialIsClearError( void );
extern int SerialIsGetError( void );

#endif //BLUEBERRIAN_SERIAL_H
