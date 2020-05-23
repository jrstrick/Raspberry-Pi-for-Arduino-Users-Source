 /*
  * button_polled
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
 * button_poled
 * This project is, in most respects, a duplicate of Larson(memorial) 
 * scanner project, save that it connects a momentary switch between 
 * BCM12 and the LED drive positive rail. I'll call the changes out 
 * where they appear.
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

/* New for button_poled: button_pin is the BCM pin number that our 
 * button is connected to. This way it can be changed easily.
 */
 #define button_pin 12

/* 
 * We're only using one namespace in this program, so it's safe to set
 * this program's default namespace to std. 
 */
using namespace std;

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
 * New for button_poled: initialize button_pin as an input, and turn on 
 * the pin's pulldown resistor.
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
	
	//Initialize Pins.
	for (c=0;c<LEDs;c++){
		//cout<<"Setting pin "<<pins[c]<<"to output\n"<<flush;
		pinMode (pins[c],OUTPUT);
		digitalWrite(pins[c],HIGH);
	}
	pinMode(button_pin,INPUT); //set button_pin's pinmode to input.
	pullUpDnControl(button_pin,PUD_DOWN);
	
	//Loop forever switching the LEDs on and off in sequence.
	while(true){
		if((bool)digitalRead(button_pin)){
			cout<< "Button is Pressed" << endl;
		}
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
