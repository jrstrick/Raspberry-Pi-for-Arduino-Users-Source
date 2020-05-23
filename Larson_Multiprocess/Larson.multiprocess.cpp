 /* Larson_multiprocess.cpp
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
 * Larson_multiprocess
 * This program implements the classic "Larson (memorial) scanner",
 * albeit a 20 pin Raspberry Pi version, with two "eyes," each running in 
 * its own pthread It //requires// a Pi with a 40 pin GPIO bus, or it 
 * won't call the right pins in wiringPi.
 * Several modifications from Larson.cpp. First, we install a signal
 * handler to turn all the LEDs off when we get an interrupt signal.
 * Next, we split the actual scanning out of main and into a function.
 * It is no longer hardcoded to run forever, only as long as running is 
 * true, which it is until a SIGINT is caught. Then we clean up the LEDs
 * and exit.
*/


/* Iostream gives us cin and cout, which we might need for debugging.
 * wiringPi is the star of this show, the software interface to the 
 * GPIO pins. pthread.h gives us the Linux thread library. It's included
 * in wiringPi too, so we don't //technically// have to include it here,
 * but it's bad business to depend on declarations in other libraries.
 * 
 * We define LEDs as 20, both to set the index of the pins array, and 
 * as the maximum index value of the loop that reads it.
 * Likewise delaymils is a constant value of how many milliseconds to
 * wait between changing LEDs.
*/

#include <iostream>
#include <csignal>
#include <wiringPi.h>
#include <unistd.h>
#define LEDs 20
#define delaymils 40

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
volatile bool running=true;

/* void scan(bool low_order_LEDs, bool start_low)
 * Parameters:
 * 	low_order_LEDs is a boolean that tells scan whether it's scanning
 * 		from the 0th LED to 10. If this parameter is false, we're scanning
 * 		from LED 10 to LED 20.
 * 	start_low is a boolean that tells scan whether to start at the 
 *  	lowest led and scan up, or the highest led and scan down. 
 * 		This only effects the //first// scan.
 * Returns nothing.
 * 
 * How it Works:
 * -------------
 * Basically the guts of main() in Larson.cpp, but we have to do some
 * processing of local variables first.
 * Initialize c at 0 for safety. 
 * 
 * Declare localLEDs and ocalzero. These are the maximum and minimum 
 * leds this particular instantiation of scan should scan between.
 * 
 * Initialize localstart_low to start_low. If it's false, we start high
 * on the first scan.
 * 
 * Set up localzero and localLEDs. 
 * If we were passed low_order_LEDs as true, 
 * 		set localzero to 0 and localLEDs to LEDs divided by 2. (This 
 * 		value is probably 10, but who knows how many LEDs those wiley 
 * 		users have hooked up?)
 * Otherwise (low_order_LEDs is false), 
 * 		set localzero to LEDs divided by 2 and localLEDs to LEDs.
 *  
 * Loop as long as running is true. It's set false by the signal handler.
 * (This means we may get as much as one full scan between the signal
 * being caught and scan exiting. That's ok.)
 * 		If localstart_low is TRUE
 * 			Scan Low to High LED #. Starting with localzero, 
 * 				turn the LED on, 
 * 				wait delaymils, 
 * 				then turn the previous LED off.
 * 		else set localstart_low TRUE.
 * 			(start_low and localstart_low only effect the first scan.)
 * 			Scan from high to low
 * 				Switch the current LED on, 
 * 				Wait delaymils, 
 * 				then switch the previous LED off. 
 */
void scan(bool low_order_LEDs,bool start_low){
	int c=0;			
	int localLEDs;
	int localzero;
	bool localstart_low=start_low;
	
	//set up localzero and localLEDs
	if (low_order_LEDs){
		localzero=0;
		localLEDs=(LEDs/2);
	}else{
		localzero=(LEDs/2);
		localLEDs=LEDs;
	}
	while(running){ //loop forever as long as running is true.
		if (localstart_low){
			//if localstart_low is true, loop from localzero to 
			// localLEDs - "scan" from low LED # to high.
			for (c=localzero;c<localLEDs;c++){
				//cout << "switching" << pins[c] <<"\n"<< flush;
				digitalWrite(pins[c],LOW);
				delay(delaymils);
				if (c>localzero) digitalWrite(pins[c-1],HIGH);
			}
		}else{
			//if localstart_low was false, we skipped the low-to-high
			//part of the scan and came here. We only want this to 
			//happen on the first scan, so when we GET here, set
			//localstart_low true. Then do the high-to-low part
			//of the scan as usual.
			localstart_low=TRUE;
		}
		
		//loop from localLEDs to localzero - scan from high LED # to low.
		for (c=localLEDs-1;c>=localzero;c--){
			//cout << "switching" << pins[c] <<"\n"<< flush;
			digitalWrite(pins[c],LOW);
			delay(delaymils);
			if (c<localLEDs)digitalWrite(pins[c+1],HIGH);
		}
	}
}

/* SIGINT_handler
 * Parameters:
 * signal_number, an integer
 * Returns: nothing.
 * 
 * How it works
 * ------------
 * Very much like a wiringpiISR interrupt handler, signal_handler is 
 * called by a system event, namely a signal being raised. In this
 * case we're looking for SIGINT, a macro for the signal we get when
 * ctrl-c is pressed in the geany run window.
 * This handler only fires if SIGINT is raised, so no decoding logic
 * is needed.
 */
void SIGINT_handler(int signal_number){
	running=false;
}

/*
 * Main()
 * Parameters: none
 * Returns: an integer to tell the system its exit status.
 * 
 * How it works:
 * -------------
 * Declare the variable process_id as type pid_t. This is an integer
 * of some kind, but what kind exactly is platform specific. We 
 * initialize that variable with a call to fork(), which starts a
 * child process identical to the parent we're in, with all the same
 * variables except process_id. The child's process id is stored in the
 * parent's process_id variable. The child's own process_id varabile
 * will be set to 0. Forked processes start at the next 
 * 
 * Initialize wiringPi. Both the parent and child processes must do this.
 * If process_id is greater than 0, we're the parent process.
 * 		Initialize the pins. Only needs to happen once - there's only
 * 		one set.
 * 		scan the low order LEDs, starting with the highest one (LED 10,
 * 		most likely. (low_order_leds true, start_low false)
 * else
 * 		We don't need to initialize the pins again, and we already
 * 		initialized wiringPi, so just scan the high order LEDs starting
 * 		with the lowest one. (low_order_leds false, start_low true) 
 * 
 * Scan only returns if the signal handler sets running to false, so
 * if scan returns, turn all the LEDs off and exit.
*/
  
int main(void){
	//connect up the signal handler to fire on SIGINT.
	signal(SIGINT,SIGINT_handler);
	
	//fork the child process and store its PID in process_id on the
	//parent process. The fork() call returns 0 to the child process.
	pid_t process_id=fork();
	
	//Initialize wiringPi
	wiringPiSetupGpio();
	
	if (process_id>0){//parent process
			//Initialize Pins.
	for (int c=0;c<LEDs;c++){
		//cout<<"Setting pin "<<pins[c]<<"to output\n"<<flush;
		pinMode (pins[c],OUTPUT);
		digitalWrite(pins[c],HIGH);
	}
		scan(true,false);
	}else{ //child process
		scan(false,true);
	}
	//If we get here, scan has exited, most likely because the signal 
	//handler has set running to false. So turn all the LEDs off and 
	//exit.
	for (int c=0;c<LEDs;c++){
		digitalWrite(pins[c],HIGH);
	}	
	return 0;
}
