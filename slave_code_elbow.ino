
/********************************************************************************************************
      SPECULUM VITAE MARK II 
      KAPTA INDUSTRIES LTD.
      
      this code is for the slave arduino 
      it recieves a pot value from the master arduino and uses it for stepper motor control
      
      written by:  Scott Henwood
                   Andrew Henwood
***********************************************************************************************************/



#include <Wire.h>                            // includes the wire library
#include<stdio.h>


const int potPin1=A7;                       // declares potPin 1 pin (encoder attached to motor)
const int dirPin=6;                         // declares pin for controlling the direction of the stepper
const int stepPin=7;
;                        // declares pin for pulsing the stepper motor
int error,angle,angle1,potVal;
int spd=0;
int del;


void setup() {

 pinMode(potPin1,INPUT);
 pinMode(dirPin,OUTPUT);
 pinMode(stepPin,OUTPUT);
  Wire.begin(3);                             // join i2c bus with address #8(must be same adress assigned on master)
  Wire.onReceive(receiveEvent);              // call function and recieve when arduino detects and i2c message
  Serial.begin(115200);                      // start serial for output
  angle=360;
}

void loop() {
   
potVal=analogRead(potPin1);                 // sets the value used for position control
angle1=potVal;

 Serial.print("angle=   ");
 Serial.print(angle);
 Serial.print("   angle1=   ");
 Serial.print(angle1);
 Serial.print("   error=   ");
 Serial.print(error);
 Serial.print("   frequency"  );
 Serial.print(del);  
 Serial.print("\n");
 
 
error=abs(angle-angle1);                    // creates an error number allowing the motor to be at rest 
  
                                            //when it becomes close to its target value 
                                            //otherwise the motor would never sit still due to interference 

if(error<10)
del=20;

else del=error*20;


 if(angle>angle1&&error>5)                 // if true will turn the motor in a specified direction
 {
   
   digitalWrite(dirPin,HIGH);
   tone(stepPin,del); 
   
   
 }
else if(angle1>angle&&error>5)             // if true wil turn the motor in the opposite direction
{
    digitalWrite(dirPin,LOW);
   tone(stepPin,del);
 }
 else{del=0;
 noTone(stepPin);
 }
}

                                              // function that executes whenever data is received from master
                                              // this function is registered as an event, see setup()
void receiveEvent(int howMany) {
    unsigned short value;                     // creates the same 16 bit integer to store the recieved data that was sent from master
    byte* x = (byte*)&value;                  // splits value into two bytes x= bits 0-7 x+1= bits 8-15
    *x = Wire.read();                         // receive byte x (lower half of int 'value')
    *(x+1) = Wire.read();                     // recieve byte x+1 (upper half of int 'value')
    angle=value;                              // makes angle equal to recieved int 'value'
  




}
