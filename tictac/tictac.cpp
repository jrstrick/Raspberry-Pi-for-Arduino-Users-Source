/*
 * tictac.cpp
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

/* tictac
 * version 1.0
 * language: c++
 * This program exists solely to demonstrate basic C++ programming 
 * concepts, like objects, classes, and methods. It is two player, so 
 * there is no programmed opponent (game theory would be too much).
*/
 
 
/* Iostream gives us cin and cout, among other things we won't
 * be using. These functions let us talk to the terminal. It also defines
 * the std:: namespace. Note that every time we call cin or cout we have
 * to tell C++ what namespace those functions are in. There are ways
 * around that, but for this first program we'll go ahead and do it 
 * explicitly.
 */
#include <iostream>


/* Class Declaration
 * ------------------------------------------------
 * 
 * Private methods:
 * ------------------------------------------------
 * check_across - checks a horizontal line for wins.
 * check_vert - checks a vertical line for wins.
 * check_diag - checks a diagonal line for wins.
 * ------------------------------------------------
 * 
 * Public Methods:
 * ------------------------------------------------
 * board - constructor. Sets all chars in board_data to empty.
 * win_check - checks to see if the player given has won.
 * player_move - interacts with the player to get coordinates 
 *      and call wincheck
 * -------------------------------------------------
 * There's no custom destructor. When we'd destruct the object, the 
 * program is terminating anyway.
 * -------------------------------------------------*/
class board{
	private:
	
	/* Private variable: board_data
	 * -----------------------------------------------
	 * What it is:
	 * -------------
	 * a two dimensional array of characters.
	 * 
	 * What it's for:
	 * -------------
	 * stores all the X or O values. Represents the board.
	 * -----------------------------------------------
	 */ 
	char board_data[3][3];
	
	/* Private method: check_across
	 * ----------------------------
	 * Parameters: int row, char player
	 * Returns: a boolean value (true or false)
	 * 
	 * How it works:
	 * ------------- 
	 * given a row and a player, start at the 0th
	 * column in that row and traverse that row of the array to the 
	 * 2nd position. If any of those chars are NOT equal to player, 
	 * return false, otherwise return true.
	 * -----------------------------------------------
	 */
	bool check_across(int row, char player){
		for (int c=0;c<=2;c++){
			if (board_data[row][c]!=player) return (false);
		}
		return(true);
	}
	
    /* Private method: check_vert
	 * ----------------------------
	 * Parameters: int col, char player
	 * Returns: a boolean value (true or false)
	 * 
	 * How it works:
	 * ------------- 
	 * Given a column and a player, start at the 0th
	 * row in that column and traverse that column vertically to the 
	 * 2nd position. If any of those chars are NOT equal to player, 
	 * return false, otherwise return true.
	 * -----------------------------------------------
	 */
	bool check_vert(int col, char player){
		for (int c=0;c<=2;c++){
			if (board_data[c][col]!=player) return(false);
		}
		return(true);
	}
	
	/* Private method: check_diag
	 * ----------------------------
	 * Parameters: bool up, char player
	 * Returns: a boolean value (true or false)
	 * 
	 * How it works:
	 * ------------- 
	 * If up is false
	 * 	Loop on c.
	 * 		Using the value of c as both row and column, check each 
	 * 		value in the board_data array at that position (c,c). If 
	 *		it's not equal to player, return false.
	 * 	If c goes past 2, return true.
	 * 
	 * Otherwise (up is true) -
	 * 	Initialize d as an int with the value 2
	 * 	Loop on c from 0 to 2
	 * 		if the char in board data at row d, column c is not equal
	 * 		to player, return false. otherwise decrement d and repeat.
	 * 		The c variable will be incremented automatically by the for
	 *  	loop.
	 * 	If c goes past 2, return true.
	 * -----------------------------------------------
	 */
	bool check_diag(bool up,char player){
		if (!up){
			for (int c=0;c<=2;c++){
				if (board_data[c][c]!=player) return(false);
			}
		}else{
			int d=2;
			for (int c=0;c<=2;c++){
				if (board_data[d][c]!=player) return(false);
				d--;
			}
		}
		return(true);
	};
	
	public:
	/* Public method: board
	 * ----------------------------
	 * Parameters: none.
	 * Returns: nothing.
	 * 
	 * How it works:
	 * ------------- 
	 * This method is the class constructor. It's called when an
	 * instance of the "board" class is created. 
	 * The functionality is classic stuff: a pair of nested loops.
	 * The outer for loop loops on c. This will be the row.
	 * 	The inner for loop loops on d. 
	 * 		Set board_data at the addresses c and d to the empty 
	 * 		character.
	 * 	increment d
	 * 	When d gets bigger than 2 increment c.
	 * When c gets bigger than 2, exit.
	 * -----------------------------------------------
	 */
	board(){
		for (int c=0;c<=2;c++){ //outer loop
			for (int d=0;d<=2;d++){//inner loop
				board_data[c][d]=(char)0; //set the element of the array
			}//end of inner loop
		}//end of outer loop
	}
	
	/* Public method: display
	 * ----------------------------
	 * Parameters: none.
	 * Returns: nothing.
	 * 
	 * How it works:
	 * ------------- 
	 * This method is all about pretty-printing the board_data array.
	 * First, we print the column number line.
	 * After that, it's a classic nested for loop to traverse the 
	 * board_data array by rows.
	 * 
	 * Of note: while this method is public (can be called by private 
	 * entities), because it is part of the board class, it can access
	 * board's private variables. It could call board's private methods
	 * too, but does not.
	 * 
	 * Loop on c
	 * 	Every time the c loop increments, we're at a new row, so 
	 * 	print the row's coordinate value
	 * 	Loop on d.
	 * 		If board_data at row c, column d is empty (which will
	 * 		make the value logically false), print three blank spaces.
	 * 		otherwise print board_data at row c column d, with a blank
	 * 		space on each side.
	 * 		if d (the column) is less than 2, print a pipe character(|)
	 * 		this forms the vertical lines in the tic tac toe board.
	 *		increment d.
	 * 	When the d loop gets past 2 and exits, print an endl. This
	 * 	ends the line so we start at the left margin for the next line.
	 *	if c is less than 2, print the horizontal line for the crosshatch.
	 * 	Increment c.
	 * When the c loop exits, we've gone through all the elements in the
	 * board_data array. Print an endl to give some whitespace.	
	 * -----------------------------------------------
	 */
	void display(){
		std::cout<<"   (0) (1) (2)"<<std::endl; //column numbers
		for(int c=0;c<=2;c++){
			std::cout<<"("<<c<<")"; //row number
			for (int d=0;d<=2;d++){
				if (!board_data[c][d]){
					std::cout<<"   "; 
				}else{
					std::cout<<" "<<board_data[c][d]<<" ";
				}
				if (d<2) std::cout<<"|";
			}//end of d loop (columns)
			std::cout<<std::endl;
			if (c<2) std::cout<<"   --- --- ---"<<std::endl;
		}//end of c loop (rows)
		std::cout<<std::endl;
	}//end of method.
	
	/* Public method: win_check
	 * ----------------------------
	 * Parameters: player, a char.
	 * Returns: a boolean (true or false).
	 * What it does:
	 * ------------- 
	 * Tic Tac Toe is a simple game. There are only 8 win conditions, so
	 * we'll just check them all. I'm playing more boolean games here.
	 * In boolean, only false plus false is false, so any true returned
	 * by any of the checks will be preserved.
	 *  
	 * Of note: While this method is public (can be called by outside
	 * (entities), because it is part of the board class, it can call 
	 * board's private methods. It could access private variables too,
	 * but it doesn't.
	 * 
	 * How it works:
	 * ------------- 
	 * Set the boolean "won" to false.
	 * Loop on c. 
	 * 	Add whatever check_across on row c returns.
	 * 	Add whatever check_vert on column c returns.
	 * Increment c.
	 * When loop c exits, check both diagonals (top left to bottom 
	 * right and bottom left to top right) and add their results to won.
	 * Return won. If any of the win conditions has occurred, it will be
	 * true.
	 * -----------------------------------------------
	 */	
	bool win_check(char player){
		bool won=false;
		for (int c=0;c<=2;c++){
			won+=check_across(c,player); 
			won+=check_vert(c,player);
		}//end of the c loop
		won+=check_diag(false,player);
		won+=check_diag(true,player);
		if (won) std::cout<<"Player "<<player<<" Won."<<std::endl;
		return(won);
	}//end of the win_check method
	
	/* Public method: player_move
	 * ----------------------------
	 * Parameters: player (a char)
	 * Returns: a boolean (true or false).
	 * 
	 * What it does:
	 * ------------- 
	 * The player_move method does two things. Given a player name, it
	 * asks the player for a row and a column for that player's move.
	 * It then checks the move to make sure it's not out of range
	 * (no values greater than 2) and that the box the player chose
	 * is not already taken.
	 * 
	 * Of note: while this method is public (can be called by private 
	 * entities), because it is part of the board class, it can access
	 * board's private variables. It could call board's private methods
	 * too, but does not.
	 * 
	 * How it works:
	 * ------------- 
	 * Set a boolean called "Valid" to false.
	 * Set integers "row" and "column" to zero
	 * Loop on valid not being true,
	 * 	Ask the player to enter coordinates.
	 * 	Input (cin) the row and the column
	 * 	If the element of board_data at row and col is empty //and//
	 *  if row is less than 3 //and// if column is less than 3:
	 * 		thank the user, set the value in board_data, and set
	 * 		valid to true.
	 * 	Otherwise tell the user that the coordinates they gave were 
	 * 	invalid.
	 * Go back to the beginning of the valid loop.
	 * If valid is true, exit from the method.
	 * -----------------------------------------------
	 */	
	void player_move(char player){
		bool valid=false;
		int row=0,col=0;
		std::cout<<"Player "<<player<<", it's your move."<<std::endl;
		std::cout<<"Enter Coordinates"<<std::endl;
		
		while (!valid){
			std::cout<<"Row: ";
			std::cin>>row;
			std::cout<<"Column: ";
			std::cin>>col;
										  //   (and)      (and)
			if ((board_data[row][col]==(char)0) && (row<3) && (col<3)){
				std::cout<<"Thank You."<<std::endl;
				board_data[row][col]=player;
				valid=true;
			}else{
				std::cout<<"Invalid Coordinates. Please try again."<<std::endl;
			} //end of if
			
		} //end of while valid is not true loop
		
	} //end of method
	
}; //end of the board class


/* main function
 * ----------------------------
 * Parameters: nothing
 * Returns: an integer (C++ requires this)
 * 
 * How it works:
 * ------------- 
 * Create the boolean "we_have_a_winner" and set it false
 * Create the char "player" and set it to X. (X always starts)
 * Instantiate an object of the board class (above) called the_board
 * Loop on we_have_a_winner being false
 * 	Call the "display" method on the_board.
 * 	Call the "player_move" method on the_board, and tell it which player.
 * 	Call the "win_check" method on the_board for the player.
 * 	If we've been dealing with the X player, set the variable player
 * 	to O. Otherwise it must be O, so set it to X.
 * End of the while we_have_a_winner is false loop, which means someone
 * won.
 * Display the final board.
 * Exit, returning zero to tell Linux there were no errors.
 * -----------------------------------------------
 */	
int main(){
	bool we_have_a_winner=false; //has anyone won?
	char player='X'; //what player is playing?
	board the_board; //instantiate the object the_board
	
	while(!we_have_a_winner){ //loop on we_have_a_winner being false
		the_board.display(); //display the board
		the_board.player_move(player); //have the player move
		we_have_a_winner=the_board.win_check(player); //see if they won

		if (player=='X'){ //if player X was playing
			player='O'; //set player to O
		}else{ 
			player='X'; //otherwise set player to X
		}//end of the if
	}//end of the loop
	
	the_board.display(); //display the final board
	return 0; //return zero, telling Linux that there were no errors
}//end of the program.


