 /*
  * button_interrupt.cpp
  * 
  * Copyright 2017 Jim Strickland <jrs@jamesrstrickland.com>
  * 
  * Standard GNU boilerplate:
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  * MA 02110-1301, USA.
  * 
 */
  
/*
 * button_interrupt
 * This program is derived from button_polled and Larson.cpp, and is 
 * used to demonstrate wiringPi's interrupt functionality. Most of the
 * button_polled code has been replaced. 
*/


/* Iostream gives us cin and cout, which we might need for debugging.
 * wiringPi is the star of this show, the software interface to the 
 * GPIO pins.
 * We define LEDs as 20, both to set the index of the pins array, and 
 * as the maximum index value of the loop that reads it.
 * Likewise delaymils is a constant value of how many milliseconds to
 * wait between changing LEDs.
*/
#include <iostream>
#include <wiringPi.h>

#define LEDs 20
#define delaymils 40

/* Set the button pin to 12. Also define the debounce delay.
 */
 
#define button_pin 12
#define button_debounce_delay 100 //milliseconds

/* Define a static int to hold the number of button presses, and a 32
 * bit unsigned integer to hold the time value of the last time the 
 * interrupt fired. This is for debouncing.
 */
volatile int button_presses=0;
volatile uint32_t last_time_interrupt_fired=0;

/* 
 * We're only using one namespace in this program, so it's safe to set
 * this program's default namespace to std. 
 */
using namespace std;

/*This is the Interrupt Service Routine that will be called when we press
 * the button. It takes no parameters and returns nothing, as must all ISRs.
 * 
 * How it works: 
 * ------------
 * First, we declare another 32 bit unsigned integer to hold the output
 * of millis(), since we'll use the value twice and we'd prefer it didn't
 * change. We store the value of millis() minus the value of 
 * the last_time_interrupt_fired static unsigned 32 bit integer in it.
 * Next, we update last_time_interrupt_fired with the current value of 
 * millis().
 * If time_since_last_interrupt is greater than the button_debounce_delay,
 * which is a preprocessor macro of some number of milliseconds, then
 * print a message on the screen and exit. If not, just exit.
 */
void button_ISR(void){
	uint32_t time_since_last_interrupt=millis()-last_time_interrupt_fired;
	last_time_interrupt_fired=millis();
	
	if (time_since_last_interrupt >button_debounce_delay){
		cout<<"Time Since Last Interrupt:"<<time_since_last_interrupt<<endl;
		cout<<"Button Pressed "<<++button_presses<<" Times."<<endl;
	}
}

/*
 * There's no good reason to use an object here, so we'll use a global 
 * variable instead. This is the array that maps the pin numbers in the 
 * order they're plugged into the LED arrays to its own index value (from 
 * 0 to LEDs.)
*/ 
int pins[LEDs]={2,3,4,14,15,18,17,27,22,23,24,10,9,11,25,8,7,1,0,5};


/*
 * Main()
 * Parameters: none
 * Returns: an integer to tell the system its exit status.
 * 
 * How it works:
 * -------------
 * Initialize the integer c at zero. C will be a counter. This isn't 
 * really necessary, since it's initialized in the for loop below, but
 * it's good practice.
 * 
 * Initialize wiringPi
 * Call the wiringPiSetupGpio() function. This function configures
 * the program's interface with the wiringPi/GPIO system. Critically, it
 * sets it up to use the Broadcom GPIO numbers instead of earlier wiringPi
 * specific pin numbers, physical pin numbers, or anything else.
 * 
 * Initialize pins
 * In a for loop, we set all the pins to output mode and high. We're
 * using them for output.
 * Note Bene: We are switching the //low// or cathode side of each LED.
 * When the GPIO is //HIGH//, the LED is //off// Only when the GPIO is 
 * //LOW// does current flow. So we set them all high at initialization
 * to turn all the LEDs off.
 * 
 * Loop Forever scanning from low to high and high to low.
 * 		Scan Low to High LED #. Starting with LED 0, 
 * 			turn the LED on, 
 * 			wait delaymils, 
 * 			then turn the previous LED off.
 
 * 		Scan High to Low by LED number. 
 * 			Switch the current LED on, 
 * 			Wait delaymils, 
 * 			then switch the previous LED off.
 * 
 * We'll never reach the return(0). 
*/

int main(void){
	int c=0;
	//Initialize WiringPi.
	wiringPiSetupGpio();
	
	//Initialize interrupt timer variable.
	last_time_interrupt_fired=millis();
	
	//Initialize Pins.
	for (c=0;c<LEDs;c++){
		//cout<<"Setting pin "<<pins[c]<<"to output\n"<<flush;
		pinMode (pins[c],OUTPUT);
		digitalWrite(pins[c],HIGH);
	}
	pinMode(button_pin,INPUT); //set button_pin's pinmode to input.
	pullUpDnControl(button_pin,PUD_DOWN); //turn the pin's pullup/pulldown off
	
	//Hook up button_ISR as an interrupt service routine on button_pin.
	wiringPiISR(button_pin,INT_EDGE_FALLING,&button_ISR);
	
	//Loop forever switching the LEDs on and off in sequence.
	while(true){
		
		//loop from 0 to LEDs - "scan" from low LED # to high.
		for (c=0;c<LEDs;c++){
			//cout << "switching" << pins[c] <<"\n"<< flush;
			digitalWrite(pins[c],LOW);
			delay(delaymils);
			if (c>0) digitalWrite(pins[c-1],HIGH);
		}
		
		//loop from LEDs to 0 - scan from high LED # to low.
		for (c=LEDs-1;c>=0;c--){
			//cout << "switching" << pins[c] <<"\n"<< flush;
			digitalWrite(pins[c],LOW);
			delay(delaymils);
			if (c<LEDs)digitalWrite(pins[c+1],HIGH);
		}
	}
	return 0;
}
