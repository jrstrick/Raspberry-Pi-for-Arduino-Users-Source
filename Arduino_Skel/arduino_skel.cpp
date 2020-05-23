#include "do_nothing.cpp"
#include <iostream>
void setup() {
  // put your setup code here, to run once:
do_nothing();
}

void loop() {
  // put your main code here, to run repeatedly:
  std::cout<<"Sketches are really C++!!!"<<std::endl;
}
int main(){
	setup();
	for(int c=0;c<5;c++){
		loop();
	}
}
