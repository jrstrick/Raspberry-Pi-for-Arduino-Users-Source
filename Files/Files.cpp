/* Files.cpp
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
/* Files.cpp
 * Copyright 2018 Jim Strickland <jrs@jamesrstrickland.com>
 * 
 * This program is a simple demonstration of writing and reading a text 
 * file. Binary files are done exactly the same way save that put and 
 * get are used instead of the string formatted << and getline 
 * directives. 
*/


#include <iostream> //As always, we need iostream for cout.
#include <fstream>  //fstream gives us file streams.
#include <string>   //Since this is a text file demo, we need strings.
#define fullpath "/home/pi/myFlash/my_test_file.txt"

using namespace std; //As always, std:: namespace

int main(){
	string line; //data has to go somewhere when we read it. 
	
	fstream file_object; //define the actual object
	
	file_object.open(fullpath,ios::out);
	//open the file for writing. 
	
	if (file_object.is_open()){
	//Sanity check: is the file open?
	
		file_object << "This text goes into the file,";
		file_object << " just like into cout."<<endl;
		//If so, write to it...
		
		cout <<"Wrote to the file."<<endl;
		//...and tell the user...
		
		file_object.close();
		//...then close the file.
		
	}else{
		cout <<"Unable to open file to write. Exiting."<<endl;
		exit(1);
		//Otherwise something is horribly wrong. Exit. 
		//(It's probably perms or ownership of the dir. on the SD card.)
	}
	
	file_object.open(fullpath,ios::in);
	//Reopen the file to read.
	
	if (file_object.is_open()){
	//Sanity check that it's open again.
	
		cout << "From the file, I read:"<<endl;
		//Tell the user.
		while (getline(file_object,line)){
			cout <<line <<endl;
		}
		//Read while there are lines to read. When the EOF (end of file)
		//flag is raised, getline will return false.
		
		file_object.clear(); //clear the EOF flag. And all other flags.
		cout <<"Cleared the EOF flag."<<endl;
		
		file_object.seekg(0,ios::beg);
		//Seek to zero bytes from the beginning of the file.
		cout <<"Used seekg to go to the beginning of the file."<<endl;
		
		while (getline(file_object,line)){
			cout <<line <<endl;
		}
		//Read while there are lines to read again.
		
		file_object.close();
		//Close the file when we're done.
	}else{
		cout <<"Unable to open file to read. Exiting."<<endl;
		exit(1);
	}
	
	return(0);
};

