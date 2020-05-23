 /* Larson_pthread.cpp
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
 * Larson_pthread
 * This program implements the classic "Larson (memorial) scanner",
 * albeit a 20 pin Raspberry Pi version, with two "eyes," each running in 
 * its own pthread It //requires// a Pi with a 40 pin GPIO bus, or it 
 * won't call the right pins in wiringPi. It's a pthread demo, basically.
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
#include <pthread.h>
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
pthread_mutex_t running_lock; //declare our mutex.

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
 * Declare localLEDs and localzero. These are the maximum and minimum 
 * leds this particular instantiation of scan should scan between.
 * Also declare local_running, so we can confine our access to running
 * to one spot.
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
 * Loop as long as local_running is true. If running is set false by
 * the signal handler, local_running will be set that way also.
 * (This means we may get as much as one full scan between the signal
 * being caught and scan exiting. That's ok.)
 * 		try the mutex lock on running_lock. If we can lock it
 * 			set local_running to running
 * 			unlock running_lock.
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
	bool local_running=true;
	
	//set up localzero and localLEDs
	if (low_order_LEDs){
		localzero=0;
		localLEDs=(LEDs/2);
	}else{
		localzero=(LEDs/2);
		localLEDs=LEDs;
	}

	while(local_running){ //loop forever as long as local_running is true.
		if (!pthread_mutex_trylock(&running_lock)){
			local_running=running;
			pthread_mutex_unlock(&running_lock);
		}
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

/* void *upper(void *vp)
 * Parameters:
 *	  void *vp, a void pointer called vp. 
 *	  Not used for anything, but the pthreads call requires it.
 * Returns: 
 *		a void pointer. 
 * 		Note bene: void pointers are not void, so we do actually have
 * 		to return a NULL pointer. 
 * 
 * These are here so the pthread functions can pass parameters to and 
 * return the end status of the thread.
 * 
 * How it works:
 * -------------
 * call scan, tell it to use the low order LEDs, and to start low.
 * return a NULL pointer.
 */
void *upper(void *vp){	
	scan(false,true);
	return(NULL);
}

/* void *lower(void *vp)
 * Parameters:
 *	  void *vp, a void pointer called vp. 
 *	  Not used for anything, but the pthreads call requires it.
 * Returns: 
 *		a void pointer. 
 * 		Note bene: void pointers are not void, so we do actually have
 * 		to return a NULL pointer. 
 * 
 * These are here so the pthread functions can pass parameters to and 
 * return the end status of the thread.
 * 
 * How it works:
 * -------------
 * call scan, tell it to use the high order LEDs, and to start low.
 * return a NULL pointer.
 */
void *lower(void *vp){
	scan(true,false);
	return(NULL);
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
	pthread_mutex_lock(&running_lock);
	running=false;
	pthread_mutex_unlock(&running_lock);
}

/*
 * Main()
 * Parameters: none
 * Returns: an integer to tell the system its exit status. We're 
 * actually using this status when we test to make sure the pthreads 
 * started properly.
 * 
 * How it works:
 * -------------
 * Set up wiringPi to use BCM GPIO numbers
 * 
 * Declare two pthread IDs, upper_thread and lower_thread.
 * 
 * Initialize the GPIO pins as outputs and set them HIGH (off)
 * 
 * Tell the user we're creating the upper thread, then call 
 * pthread_create and create the thread. Notify the user and exit if 
 * we fail. This thread will light the LEDs from 10-20.
 * 
 * Tell the user we're creating the lower thread, then call 
 * pthread_create and create it, too. As before, if we fail, tell the
 * user and exit. This thread will light the LEDs from 0-10.
 * 
 * Make a pithy comment to the user so they know the main thread is
 * proceeding on its own.
 * 
 * For our main thread, loop as long as running is true. Not much else
 * for the main thread to do. We could have processed one of the "eyes"
 * here, obviously.  
 * Check running every 100ms as long as it's true. If it's false, go
 * through all the pins again and set them HIGH to turn them off. Then
 * exit the process which will take the pthreads we created with it.
 */
  
int main(void){
	//hook up the SIGINT signal handler.
	signal(SIGINT,SIGINT_handler);
	
	//Initialize wiringPi
	wiringPiSetupGpio();
	
	//Declare our pthread ID data structures
	pthread_t upper_thread;
	pthread_t lower_thread;
	
	//Initialize the running_lock mutex
	pthread_mutex_init(&running_lock,NULL);
	
	//Initialize Pins.
	for (int c=0;c<LEDs;c++){
		//cout<<"Setting pin "<<pins[c]<<"to output\n"<<flush;
		pinMode (pins[c],OUTPUT);
		digitalWrite(pins[c],HIGH);
	}
	
	//Start the upper thread.
	cout<<"Creating Thread Upper\n"<<flush;
	if(pthread_create(&upper_thread,NULL,upper,NULL)){
		cout<<"Error Creating thread.\n"<<flush;
		return 1;
	}
	
	//Start the lower thread.
	cout<<"Creating Thread Lower\n";
	if(pthread_create(&lower_thread,NULL,lower,NULL)){
		cout<<"Error Creating thread.\n"<<flush;
		return 1;
	}
	
	//Send a message to the user from the main thread (this thread)
	cout<<"It's really hard to see like this...\n"<<flush;
	
	//Join the lower thread and block (sit here) until it exits. Then we
	//cancel the upper thread explicitly and fall through to the LED 
	//cleanup code. Could we have just monitored running from the main
	//thread and canceled both upper_thread and lower_thread from here
	//when it changed? Yes. But it makes a better demo this way.
	pthread_join(lower_thread,NULL);
	pthread_cancel(upper_thread);

	//Turn off all the LEDs by setting their pins HIGH.
	for (int c=0;c<LEDs;c++){
		digitalWrite(pins[c],HIGH);
	}
	
	//Tell the user we're done and exit the whole process,
	cout<<"Done.\n"<<flush;
	return 0;
}
