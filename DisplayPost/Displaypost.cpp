 /*
  * Displaypost.cpp
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
 * Displaypost.cpp
 * This program lets you connect your Pi to a remote website on the
 * public internet, reads a number of lines from its front page, and 
 * displays them in binary on the larson.cpp LED array.
 * It implements two classes: a gpio class that does most of what
 * larson.cpp did, and a socket_class class that contains the socket
 * itself and all the code we need to go from an internet address to 
 * reading and writing from a socket. The socket_class class assumes
 * we will only ever want SOCK_STREAM connections (tcp), but it is 
 * compatible with IPv4 and IPv6 both. We're using the modern addrinfo
 * struct instead of the old school ipv4 and ipv6 specific structs,
 * and this means that getaddrinfo() does most of the heavy lifting 
 * for us determining what version of IP we're talking to and so on.
*/

#include <iostream> //gives us cout, especially.
#include <wiringPi.h> //access to the GPIO pins.
#include <math.h> //exponent function pow() lives here.
#include <string> //std::strings
#include <csignal> //signal handlers need this.
#include <unistd.h> //NULL pointer definition, general POSIX compliance.

#define LEDs 20
#define delaymils 100
#define buffer_length 150

//#define debug_messages 1  
//We don't want the debug messages this time around. We have to be
//much more careful with our output lest we confuse the server, the
//browser, or both.

bool running=true; //clear the flag our SIGINT handler uses for 
//cleaning up the LEDs after use when we terminate the program.

void SIGINT_handler(int signal_number){
	running=false;
	//handle when we throw the program a SIGINT. Which we probably 
	//can't in this version.
}

/* 
 * We're only using one namespace in this program, so it's safe to set
 * this program's default namespace to std. 
 */
using namespace std;

/* gpio_class declaration
 * -------------------------------------------------------------------
 * Objects of this class represent the GPIO port with a public function
 * called "write" which lets outside functions send c++ strings to the
 * GPIO port for display on the LEDs.
 * -------------------------------------------------------------------
 * Private members:
 * ===================================================================
 * int pins[]					:Variable
 * 								This private variable is an array
 * 								that maps the GPIO numbers to their
 * 								position in the 20 LED array we built
 * 								for the Larson project, from LED[0] to 
 * 								LED[LEDs -1]. Assuming LEDs is defined
 * 								as 20, that's LED0 to LED19.
* -------------------------------------------------------------------
 * gpio_write() 
 * This private method accepts an 8 bit value (usually a character), 
 * and lights the appropriate LEDs (from right to left).
 * Takes an 8 bit unsigned integer as a parameter. (A char would
 * probably work, but they're not explicitly 8 bit unsigned datatypes.)
 * Returns nothing.
 * How it Works:
 * Take an 8 bit value in "data".
 * 
 * Declare a local 8 bit unsigned integer (uint8_t) called "mask."
 * 
 * Clear the pins by calling the clear_pins() public function.
 * 
 * Iterate on c from 0 to 7.
 * 		Set mask equal to 2^c - that is, the cth power of 2. (We're
 * 		actually using the floating point pow function, and casting
 * 		the inputs and results back and forth to ints.)
 * 		Since a bit is a power of two, this sets one and only one
 * 		bit in mask to 1.
 * 
 * 		If the value of data bit-wise anded with the value of mask
 * 		is not 0 (false) then data must have that bit set. Turn on the
 * 		appropriate LED.
 * ------------------------------------------------------------------	
 * Public Members:
 * ===================================================================
 * We use the default constructor and destructor. We don't want
 * to initialize wiringPi in the constructor because if we have 
 * multiple instances of this class, we put wiringPi in an unknown
 * state. (It SHOULD do nothing, but if the protection code fails to
 * account for our call, it's a fatal error.)
 * 
 * gpio_write_string()		:Method
 * 							This public method takes a std::string
 * 							called the_string, iterates through its data 
 * 							one character at a time and calls 
 * 							gpio_write() with each character, pausing 
 * 							after each one for delaymils milliseconds. 
 * 							It also prints out the string's data and
 * 							each letter as it's sent to gpio_write.
 * 							This method takes a std::string and returns 
 * 							nothing.
 * How it works:
 * ------------
 * Take a C++ string //object// called "the_string" as a parameter.
 * Declare an integer string_length and set it to the length attribute 
 * of the_string.
 * Print the string to the terminal.
 * Iterate on c from 0 to string_length-1
 * 		Print the character returned by the at() method of the_string at
 * 		position c.
 * 		Call gpio_write() with the character returned by the at() 
 * 		method of the_string.
 * 		delay delaymils milliseconds.
* -------------------------------------------------------------------
 * clear_pins()				:Method
 *							This public method iterates through the 
 * 							pins[] array attribute and switches all the 
 * 							pins HIGH, turning the LEDs off.
 * 							This method returns nothing and takes no 
 * 							parameters.
 * How it works:
 * ------------
 * Iterate on c from 0 to LEDs -1 (from 0 to 19 most likely).
 * 		Call pinMode with the value of pins[] at the c position to
 * 		set the pin as an output. Only needs to be done once, but
 * 		additional calls won't hurt anything.
 * 		Call digitalWrite with the value of pins at the c position
 * 		to set the pin HIGH, which raises the negative side of the LED
 * 		to 3.3v, equal with the positive side, thus switching it off.
* -------------------------------------------------------------------				
 */
class gpio_class {
	private:
 // ===================================================================	
	int pins[LEDs]={2,3,4,14,15,18,17,27,22,23,24,10,9,11,25,8,7,1,0,5};
 //	-------------------------------------------------------------------
	void gpio_write(uint8_t data){ //send data to the GPIO pins.
		uint8_t mask=0;  //all the bits of mask start off as 0s.
		clear_pins(); //make sure nothing is displayed already.
		for (int c=0;c<8;c++){ //for 8 bits, we'll get 2^c.  
			mask=(uint8_t)pow(2.0,(float)c); //and switch on one bit only.
			if (data & mask){ //and that one bit with data
				digitalWrite(pins[8-c],LOW);  //if data had that bit set
			}                                 //turn on that led.
		}	
	}
 // -------------------------------------------------------------------
	public:
 // ===================================================================
	void gpio_write_string(string the_string){ //Write strings LEDs.
		int string_length=the_string.length();//store this number in an
										      //int so we don't call the
											 //function as much.
		#ifdef debug_messages
			cout<<"Writing this string: "<<the_string<<endl;
		#endif
		
		for (int c=0;c<string_length;c++){
			if (!running) return; //if our sigint handler fired, exit.
			
			#ifdef debug_messages
				cout<<"C is: "<<c<<" Character is: "<<the_string.at(c)<<endl;
			#endif
			
			gpio_write(the_string.at(c)); //send each character to 
										  //gpio_write().
			delay(delaymils);			 //wait between characters.
		}
	};
	
 // -------------------------------------------------------------------
	void clear_pins(void){
		for (int c=0;c<LEDs;c++){ //iterate through all the pins.
			pinMode (pins[c],OUTPUT); //set them as OUTPUTS
			digitalWrite(pins[c],HIGH); //And turn 
		}
	};
 // -------------------------------------------------------------------
};//end of gpio_class

//clean out all the stuff the web server puts into a string during post.
string parse_cgi(string instring,string field_name){
	string temp=instring;
	if (temp.find(field_name)!=temp.npos){
		//if temp.find(field_name)!=string::npos, we found something.
		//We're using the string class's npos constant here. See below.
		temp.replace(temp.find(field_name+"="),
					field_name.length()+1,
					"");
	}
	//use the replace() function of std::strings to remove the field
	//name and the = sign that comes after it.
	
	for (string::iterator c=temp.begin();c<=temp.end();c++){
		//iterator is a datatype defined in std::strings. By using the
		//scope resolution operator, we can use the type here, outside
		//a string object (But it only makes sense in operations on
		//a string object.)
		if(*c=='+'){
			*c=' ';
		}
			//An iterator struct is a pointer.   
			//If whatever address c points at contains a "+" character...
			//set whatever address c points at to contain a blank space.
			
	} //Ever wonder what pointer arithmatic is? We just did some.
	
	return temp; //return the modified string.
}
  //We could, and perhaps should, subclass std::string and make
  //parse_cgi part of it, but it doesn't really buy us much clarity-wise.
  //Basically it would allow us to attach our own member function to string
  //and use all its public members. Private members would still be off limits.

int main(void){
	string message;  //declare our message string
	wiringPiSetupGpio(); //setup the GPIO system to use GPIO pin #s.
	
		//connect up the signal handler to fire on SIGINT.
	signal(SIGINT,SIGINT_handler);

 	gpio_class gpio; //instantiate our gpio_class object. 
	gpio.clear_pins(); //use the gpio_class method clear_pins().
	
	getline(cin,message); //read stdin (Standard command line input) for
	//text to put into the std::string message. The http server puts any
	//messages sent from browser to server in posts into the cgi as 
	//on stdin.
	
	message=parse_cgi(message,"in_text"); //Grind out the stuff that post
	//put in our input string.
	
	cout << "Content-type:text/html\n\n"<<endl;
	cout <<"<p>Wrote: \""<<message<<"\" to GPIO.</p>"<<endl;
	//Display the text, along with a content type so the server knows what
	//it's sending the viewer.
	
	gpio.gpio_write_string(message);
	//GPIO_write the string, in binary, on the LEDs.

	gpio.clear_pins(); //turn all the LEDs off.
	return 0; //exit normally.
}; //End of program
