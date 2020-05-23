 /*
  * socket.cpp
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
 * Socket.cpp
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
#include <sys/socket.h> //the socket library.
#include <netdb.h> //addrinfo struct, plus a bunch of defines.
#include <unistd.h> //NULL pointer definition, general POSIX compliance.

#define LEDs 20
#define delaymils 100
#define buffer_length 150

#define debug_messages 1

bool running=true;

void SIGINT_handler(int signal_number){
	running=false;
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

/* socket_class declaration
 * --------------------------------------------------------------------
 * The class socket_class contains the entire mechanism for connecting
 * to and exchanging data with an internet host. It makes a few
 * assumptions: first, that we'll always be creating TCP connections
 * (SOCK_STREAMS). Second, it assumes that DNS always works.
 * This class does understand IPv6 as well as IPv4 and impliments
 * as neat a class as possible to do both.
 * -------------------------------------------------------------------
 * Private
 * ===================================================================
 * file_descrptor	: Variable.
 * 					  An integer, the file descriptor of whatever
 * 				   	  socket we use.
 * -------------------------------------------------------------------
 * exit_error		: Method.
 * 					  accept an error message, display it, and 
 * 					  terminate the program with error status.
 * 					  Accepts a std::string. Returns nothing.
 * How it works
 * ------------ 		
 * 	Accept a std::string into the variable msg. 
 *	Display the string with cout.
 * 	Call the exit function with a status of 1. 
 * -------------------------------------------------------------------
 * dns_lookup		: Method
 * 					  Accepts a string containing an internet address
 * 					  and an integer with a port number. Returns a 
 * 					  pointer to an addrinfo linked list containing
 * 					  address information, IP version information,
 * 					  port information, and so on. Everything 
 * 					  connect() needs to function.
 * 					  Accepts a string and an integer. Returns
 * 					  an addrinfo *.
 * How it works
 * ------------
 * 
 * 	Declare a pointer called server_info to an adderinfo data structure.
 * 
 *  Declare an adderinfo datastructure called hints.
 *  (Hints is used to filter the output of the DNS lookup.)
 * 
 *  Set the hints.ai_socktype member to SOCK_STREAM. We only want to see
 *  TCP addresses.
 * 
 *  Set all other fields in hints to 0, which will cause no filtering
 *  on those fields.
 * 
 *  call the to_string function with port as its parameter and set
 *  the string text_port to the result. We need the port as a string
 *  containing the letters "80" rather than the integer value 80.
 * 
 *  Send output to the user telling them what text_port is set to.
 * 
 *  Do the actual dns lookup with getaddrinfo. Pass that function the 
 *  c string version of text_address and text_port, and the address
 *  of the hints addrinfo struct. Also pass in the address of the pointer
 *  server_info_pointer, so the //value// of server_info_pointer can 
 *  be set to point at the linked list returned by getaddrinfo.
 *  
 * If the dns lookup is successful, 
 * 		declare an addrinfo pointer temp_addr,
 * 		and a char array host, with 256 chars of space. It would have been
 * 		nice to use a string here, but getnameinfo absolutely refused 
 * 		to deal with them. 
 * 
 *		Iterate through the linked list that getaddrinfo returned with a 
 * 		for loop on the ptr, a pointer to adderinfo structs. When ptr is
 * 		NULL, stop. Iterate ptr by setting it to the .ai_next field of
 *	    the addrinfo struct ptr currently points at.
 * 
 * 			Because dereferncing pointers complicates reading the code, 
 * 			we declare temp_addr as an adder_info struct, and set that 
 * 			to the struct that ptr is currently pointing at.
 * 
 *	 		Next, we call getnameinfo on tempaddr,ai_adr,temp_addr.
 * 			aiaddrlen,pass it the host char array so it can fill it in, 
 * 			tell it how big the host array is,decline to pass it flags, 
 * 			and tell it we want the numeric hostname. 
 * 
 *	 		Display the hostname with cout. Don't pass endl yet. 
 * 			There's more that goes in this output line.
 * 
 *	 		If tempaddr_aifamily is AF_INET6, 
 * 				then we must be dealing with an IPv6 address. Tell the 
 * 				user so. 
 *				Otherwise tell them it's an IPv4 address.
 * 
 *			Now write the endl.
 *
 *  	Go back to the top of the for loop unless we're done.
 * 
 * 		Having exited the for loop, return server_info_ptr, exiting the
 * 		method.
 * 
 * If we reach this point, the getaddrinfo call returned something 
 * besides 0. Remember we tested that clear up at the top? If it did that
 * we had a DNS error, and can't proceed.
 * call the exit_error() function and tell the user DNS failed. Terminate
 * the whole program.
 * 
 * We can't ever get here, but we have to tell C++ that we're returning 
 * data, or it will complain.
 * -------------------------------------------------------------------
 * 
 * Public
 * ===================================================================
 * connect_socket		:Method:
 * 						 This method takes a std::string address and
 * 						 an integer port, then does a DNS lookup on
 * 				`		 the address, and uses the information that
 * 						 returns to create a socket with the right
 * 						 configuration and connect it to the address.
 * 						 This method takes a std::string address and
 * 						 an integer port and returns nothing.
 * How it Works
 * ------------
 * 	Take the string parameter address and the int parameter port.
 * 
 * 	Declare a 256 char array to hold text addresses, since 
 * 	getnameinfo refuses to work with std::strings.
 * 
 * 	Declare a pointer named dns_results and point it to the results
 *  returned by dnslookup on the address and port we were given.
 * 
 * Declare an addrinfo struct called temp_addr, because dereferencing
 * pointers constantly is a nuisance.
 * 
 * As with dnslookup, iterate on the adderinfo pointer ptr from
 * dns_results to NULL.
 * 
 * 		Set temp_addr equal to the struct pointed at by ptr. Saves
 * 		a lot of dereferencing pointers.
 * 		
 * 		Call getnameinfo on temp_addr.ai_addr,temp_addr.ai_addrlen,
 * 		host, the size of host, no flags, and request a numeric host.
 * 
 * 		Now that host[] is set, use it to tell the user which address
 * 		we're trying.
 * 
 * 		Create the socket by setting file_descriptor to the output of
 * 		the socket() function called with temp_addr.aifamily, SOCK_STREAM,
 * 		and 0. temp_addr.aifamily will be AF_INET for IPv4 and AF_INET6
 * 		for IPv6. So our socket() call will work for either one.
 * 
 * 		If file_descriptor is -1, the socket didn't create.
 * 			exit program on error.
 * 
 * 		Call connect() on the socket by passing connect the socket's
 * 		file_descriptor,the address as stored in temp_addr.ai_addr
 * 		(which includes the port) and the address length stored in 
 * 		temp_addr.aiadderlen.
 * 
 * 		If we're successful (connect returns 0),
 * 			tell the user and  
 * 			exit the for loop with break.
 * 
 * 		otherwise close the socket.
 * 	Go back to the top of the for loop and try the next address.
 * 
 * When we get here. either the for loop has exited and we've had no
 * successful connections, or we've exited the for loop with the break
 * on a successful connection. Either way, free the linked list that 
 * dns_lookup originally returned so as not to waste memory.
 * -------------------------------------------------------------------
 * read_socket() 		:Method
 * 						 This method reads up to buffer_length
 * 						 characters from the socket, which were received
 * 						 from the remote host, and returns them in a
 * 						 std::string.
 * 						 It takes no parameters and returns a
 * 						 std::string.
 * How it works
 * ------------
 * Declare a char array of buffer_length called from_server, because 
 * recv doesn't like std::strings.
 * 
 * Declare an integer called bytes, and set it to the output of the
 * recv() function, which we pass the socket's id (file_descriptor),
 * the from_server char array, buffer_length -1 (because strings
 * always have a null terminator attached)  and no flags.
 * 
 * If bytes is less than zero
 * 		exit on error. Receive has failed.
 * 
 * If we reach this point, the recv call worked. tell the user
 * how many bytes we got from the server.
 * 
 * load from_server into a std::string and return that std::string.
 * This contains the buffer_length characters the remote host sent.
 * -------------------------------------------------------------------
 * write_socket()		:Method
 * 						This method takes a std::string and writes
 * 						its contents to the socket, sending them to
 * 						the remote host.
 * 						It takes a std::string parameter and returns 
 * 						nothing.
 * How it Works
 * ------------
 * Accept a std::string into the variable text.
 * 
 * declare an integer, bytes, and set it to the output of a call to
 * the send() function, which we pass the socket's file_descriptor. We 
 * next pass the results of the std::string text's c_str() member 
 * function, the results of the std::string text's length() function,
 * and 0 flags.
 * 
 * If bytes is less than zero
 * 		exit on error - the write failed.
 * 
 * If we reach this point, tell the user how many bytes we sent. 
 * -------------------------------------------------------------------
 * close_socket()		:Method
 * 						Closes the socket.
 * 						Takes no parameters, returns nothing.
 * How it Works
 * ------------
 * Call the close function with our socket's file descriptor, set in
 * the class private variable file_descriptor.
 * ------------------------------------------------------------------- 
 */ 
 
class socket_class {
	private:
 // ===================================================================
	int file_descriptor=0; 		//The all-important name of our socket.
 // -------------------------------------------------------------------
	void exit_error(string msg){ //display the message and terminate
		cout <<msg<<endl;		//the program. Something's gone wrong.
		exit(1);
	}
 // -------------------------------------------------------------------
	
	//look up the text address and return critical data: the ip address,
	//what kind of address it is, etc, in an adderinfo struct.
	
	addrinfo *dns_lookup(string text_address,int port){
		//declare variables
		addrinfo *server_info_ptr;
		
		//declare and set up hints
		addrinfo hints;
		hints.ai_socktype=SOCK_STREAM;
		hints.ai_family=0;
		hints.ai_protocol=0;
		hints.ai_flags=0;
		
		//declare and load the text_port string.
		string text_port=to_string(port);
		
		#ifdef debug_messages
			cout<<"Using port# "<<text_port<<endl;
		#endif
		
		if (getaddrinfo(text_address.c_str(),
						text_port.c_str(),
						&hints,
						&server_info_ptr)==0){
			#ifdef debug_messages
				addrinfo temp_addr;
				char host[256]; //buffer for host names.
				
				for (addrinfo *ptr=server_info_ptr;ptr!=NULL;ptr=(*ptr).ai_next){
					temp_addr=*ptr;
					getnameinfo(temp_addr.ai_addr,
								temp_addr.ai_addrlen,
								host,
								sizeof(host),
								NULL,
								0,
								NI_NUMERICHOST);

					cout<<"Found SOCK_STREAM address: "<<host;
					if (temp_addr.ai_family==AF_INET6){
						cout <<" IPv6";
					}else{ //equal to AF_INET
						cout<<" IPv4";
					}
					cout<<"."<<endl;
				}
			#endif

			return server_info_ptr;
		}else{
			exit_error("DNS Failed.");
			return server_info_ptr;
		}		
	}; //end of dns_lookup.
 // -------------------------------------------------------------------	
	 
	public:
 // ===================================================================
	void connect_socket(string address,int port){ //creates and connects socket.
		#ifdef debug_messages
			char host[256]; //buffer for host address text.
		#endif
		
		addrinfo *dns_results_ptr=dns_lookup(address,port);
		addrinfo temp_addr; 
		
		for (addrinfo *ptr=dns_results_ptr;ptr!=NULL;ptr=(*ptr).ai_next){
			temp_addr=*ptr;
						
			//Text-ify the address and tell the user we're trying it.
			#ifdef debug_messages
				getnameinfo(temp_addr.ai_addr,
							temp_addr.ai_addrlen,
							host,
							sizeof(host),
							NULL,
							0,
							NI_NUMERICHOST);
				cout<<"Trying: "<<host<<endl;
			#endif
			
			//create socket.
			file_descriptor=socket(temp_addr.ai_family,
								   SOCK_STREAM,
								   0);
			if (file_descriptor==-1){ //socket returns -1 on fail.
				exit_error("Unable to create socket.");
			}
			
			//connect socket.			
			 if (connect(file_descriptor,
						temp_addr.ai_addr,
						temp_addr.ai_addrlen)==0){ 
				 
				#ifdef debug_messages
					cout<<"Connected!"<<endl;
				#endif
				
				break;
			}else{ //connect returns -1 on failures.
				close(file_descriptor);
			}
		}
		freeaddrinfo(dns_results_ptr); //clear memory used by dns results.
	}; //end of connect_socket.
 // -------------------------------------------------------------------
	string read_socket(){
		char from_server[buffer_length]=""; //recv hates std::string.
		//also, as with all arrays, from_server is implicitly a pointer.
								// like here.
		int bytes=0;			//	VVV 
		bytes=recv(file_descriptor,from_server,buffer_length-1,0);
		if (bytes<0){ //recv returns -1 on fails.
			exit_error("Error on Receive.");
		};
		
		#ifdef debug_messages
			cout<<"Received "<<bytes<<" bytes from server."<<endl;
		#endif
		
		return string(from_server); //calls the constructor of an unnamed 
									//std::string.
	}; //end of read_socket
 // -------------------------------------------------------------------	
	void write_socket(string text){
		int bytes=send(file_descriptor,text.c_str(),text.length(),0);
		if (bytes<0){ //send returns -1 on fails.
			exit_error("Error on Send.");
		};
		
		#ifdef debug_messages
			cout<<"Sent "<<bytes<<" bytes to server."<<endl;
		#endif
	}; //end of write_socket
 // -------------------------------------------------------------------	
	void close_socket(){ //just a wrapper for the close() function.
		close(file_descriptor);
	}; //end of close_socket.
 // -------------------------------------------------------------------
}; //end of socket_class.


int main(void){
	string message="";  //declare our message string
	int number_of_lines=0; //how many lines to read.
	string target_address; //what address should we use?
	string http_request=""; //what should we send the http server?
	wiringPiSetupGpio(); //setup the GPIO system to use GPIO pin #s.
	
		//connect up the signal handler to fire on SIGINT.
	signal(SIGINT,SIGINT_handler);

	cout<<"What address should I connect to?"<<endl;
	getline(cin,target_address);
	cout<<"How many lines should I read?"<<endl;
	cin>>number_of_lines;
	
	//http requests have to be sent quickly, or the server times out.
	//here, we're building a simple request to send us the website's 
	//top level index file. You could put any path in here, however.
	cout <<"Building HTTP request."<<endl;
	http_request="GET http://"+target_address+"/index.html";
	http_request+=" HTTP/1.1\r\nhost:"+target_address+"\r\n\r\n";
	
	cout<<"Setting up GPIO bus object."<<endl;
 	gpio_class gpio; //instantiate our gpio_class object. 
 	
	cout<<"Clearing GPIO pins."<<endl;
	gpio.clear_pins(); //use the gpio_class method clear_pins().
	
	cout<<"Setting up socket object."<<endl;
	socket_class socket; //instantiate our socket_class object.
	
	cout<<"Connecting socket to "<<target_address<<"on port 80."<<endl;
	socket.connect_socket(target_address,80); //connect our 
	//socket object to the target address the user specified, on port 80.
	//port 80 is the standard for http (web) servers.
	
	cout <<"Sending HTTP request: "+http_request<<endl;
	socket.write_socket(http_request);
	socket.write_socket("\r\n"); //send our request and an extra linefeed
	//to the http server at that address.
	
	for (int c=0;c<number_of_lines;c++){ //iterate on c for all the lines
		if (!running) break; //if our sigint handler fired, break.
		
		message=socket.read_socket(); //read from the socket into message
		cout<<"Received: "<<message<<"."<<endl; //show message
		gpio.gpio_write_string(message); //gpio_write_string message
		//so all the bytes of the lines wind up displayed on the LEDs.
	};
	socket.close_socket(); //close the socket. You only get so many,
	//so clean up after yourself.

	gpio.clear_pins(); //turn all the LEDs off.
	return 0; //exit normally.
}; //End of program
