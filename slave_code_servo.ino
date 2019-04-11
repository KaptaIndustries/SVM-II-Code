
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
#include<Servo.h>
Servo myservo;


 const int modePin=5; 
 const int trim_pot=A3;
 
int angle,servang;
int trim_p,trim_v;




void setup() {

 
  Wire.begin(6);                             // join i2c bus with address #8(must be same adress assigned on master)
  Wire.onReceive(receiveEvent);              // call function and recieve when arduino detects and i2c message
  Serial.begin(115200);                      // start serial for output
  myservo.attach(4);                         // attach servo to pin 4
  delay(15);
  pinMode(modePin,INPUT_PULLUP);
  pinMode(trim_pot,INPUT);
  //angle=360;
  
 
}

void loop() {
   
trim_p=analogRead(trim_pot);          // maps trim pot to a value between 180-600
trim_v=map(trim_p,0,1023,180,600);  
  if(digitalRead(modePin)==HIGH)      // if true disable servo motor for recording
{
  myservo.detach();
  Serial.println ("recording");
}
  
  else
  {
    myservo.attach(4);                // re-attach servo if true
    Serial.print("running       ");
    Serial.print(angle);
    Serial.print("    ");
    
    
    servang=map(angle-100,0,1023,0,trim_v);   // map recieved value using trim pot value to account for error
    Serial.print(servang);
    Serial.print("\n");
    myservo.write(servang);                   // write angle to servo
    
    
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
