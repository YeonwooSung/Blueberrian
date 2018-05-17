/**********************************************************************
 * The header file for the gpio stuff that will be used in the ezboot.*
 * Author: Yeonwoo Sung                                               *
 **********************************************************************/

#ifndef BLUEBERRIAN_GPIO_H
#define BLUEBERRIAN_GPIO_H

#define	LED_0			GPIO_bit(2)
#define	LED_1			GPIO_bit(3)
#define	LED_2			GPIO_bit(4)
#define	LED_3			GPIO_bit(5)	//

#define	LED_OFF			0
#define	LED_ON			1

void  set_GPIO_mode(int gpio_mode);
void  GPIOInit(void); // Initialise the GPIO status.
void  GPIO_SetLED( int LedIndex, int value ); //Control the LED connected to the GPIO

#endif //BLUEBERRIAN_GPIO_H
