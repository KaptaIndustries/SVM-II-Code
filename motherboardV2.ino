#include <LiquidCrystal.h>


#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <stdio.h>

File sensorData;
const int rs = 31, en = 33, d4 = 35, d5 = 37, d6 = 39, d7 = 41;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/**********************************************************************************************
  CODE FOR RECORD
 ********************************************************************************************/
const int potpin1 = A0;       // base encoder
const int potpin2 = A1;       // shoulder encoder
const int potpin3 = A2;       // elbow encoder
const int potpin4 = A3;       //twist encoder
const int potpin5 = A4;       //wrist encoder
const int potpin6 = A5;       //calw encoder
const int button = 30;        //red button
const int button1 = 34;       //green button
const int hmeButton = 38;     //white button
const int red_led = 32;       //red button led
const int green_led = 36;     //green button led
const int white_led = 40;     //white button led
const int modePin = 14;       // mode pin controlling enable and disable of drivers
unsigned short red = 0, green = 0;  // variables used to store the button presses

unsigned short homePos[6]={510,416,530,355,367,125}; //int array for manually selected homing values *change as required (left = base, right = claw)*

String dataString = ""; // holds the data to be written to the SD card
unsigned short sensorReading1 = 0; // value read from pot 1              //base
unsigned short sensorReading2 = 0; // value read from pot 2              // shoulder
unsigned short sensorReading3 = 0; // value read from pot 3              // elbow
unsigned short sensorReading4 = 0; // value read from servo pot 1        // twist wrist
unsigned short sensorReading5 = 0; // value read from servo pot 2        // pitch wrist
unsigned short sensorReading6 = 0; // value read from servo pot 3        //claw
String l_line = "";                // holds the char string retrieved from the SD card






/***********************************************************************************************
  SETUP
  The circuit:
  SD CARD ATTACHED TO SPI BUS AS FOLLOWS:
** MOSI - pin 51
** MISO - pin 50
** SCK - pin 52
** CS - pin 53

LCD ATTACHES AS FOLLOWS
**RS = 31 
**EN = 33
**D4 = 35
**D5 = 37
**D6 = 39
**D7 = 41

** red_button - pin 30
** green_button - pin 34
**red_led - pin - 32
**green_led - pin 36

**potpin - A0-A5 starting from bottom of arm to top A0 = base/ A5 = claw
**SDA - pin 20 // green wire
**SDL - pin 21 // yellow wire
 **********************************************************************************************/
void setup()
{
  lcd.begin(40, 2);
  pinMode(30, INPUT_PULLUP);    // enable all of the pins to appropriate states
  pinMode(34, INPUT_PULLUP);
  pinMode(38,INPUT_PULLUP);
  pinMode(32, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(40,OUTPUT);
  pinMode(potpin1, INPUT);
  pinMode(potpin2, INPUT);
  pinMode(potpin3, INPUT);
  pinMode(potpin4, INPUT);
  pinMode(potpin5, INPUT);
  pinMode(potpin6, INPUT);
  


  
  Wire.begin();                                             //opens i2c bus
  Serial.begin(115200);                                     // Open serial communications

  lcd.setCursor(14, 0);
  lcd.print("POWERING UP");
  lcd.setCursor(14, 1);
  lcd.print("PLEASE WAIT");
  delay(1500);
  lcd.setCursor(9, 1);
  lcd.print("INITIALIZING SD CARD...");                // verify SD connection
  delay(1500);

  if (!SD.begin(53))
  {
    lcd.clear();
    lcd.setCursor(9, 0);
    lcd.print("INITIALIZATION FAILED!");              // verification failed alert SD card not present
    lcd.setCursor(13, 1);
    lcd.print("CHECK SD CARD");
    while (1);
  }
  lcd.setCursor(8, 1);
  lcd.print("INITIALIZATION COMPLETE    ");                 // verification complete
  delay(1500);
  lcd.clear();

}





/*********************************************************************************************************
  WIRE SUBROUTINE
 ***********************************************************************************************************/

void wireTrans(int address, unsigned short value)
{
  byte* x = (byte*)&value;                /*treats value as 2 bytes
                                             x is lower byte of value bits 0-7
                                             x+1 is higher byte of value bits 8-15*/

  Wire.beginTransmission(address);        // transmit to device 
  Wire.write(*x);                       // sends lower byte of pot value first 8 bits (0-7)
  Wire.write (*(x + 1));                // sends high value of pot value last 8 bits (8-15)
  Serial.print("sending   ");          // used to test if the arduino is sending the code out to the slave
  Serial.println(address);
  Wire.endTransmission();                 // stop transmitting

}







/*********************************************************************************************************
  SAVE DATA SUBROUTINE
 ***********************************************************************************************************/
void saveData()
{
  sensorData = SD.open("test.csv", FILE_WRITE);   //opens the file on the SD card used to store values

                                                  // if the file opened okay, write to it:
  if (sensorData) {
    Serial.print("Writing to test.txt...");
    sensorData.println(dataString);               // write the data string of all the recorded values to the SD
                                                  // close the file:
    sensorData.close();
    Serial.println("done.");
  } else {
                                                  // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
}



/*********************************************************************************************************
  WAIT FOR BUTTON PRESS SUBROUTINE
 ***********************************************************************************************************/
void wait_for_one_press()           // does nothing until a button is pressed
{
  while (digitalRead(button) == 0)  // senses button one is pressed and increments red before terminating
  {
    red++;
    break;
  }

  while (digitalRead(button1) == 0) // senses button two is pressed and increments green before terminating
  {
    green++;
    break;
  }
  while (digitalRead(button) == 0)    // turns on the red led to show press has been recieved
                                      // also functions as error handler incase button is held to long
  {
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, LOW);
    digitalWrite(white_led,LOW);
    if (red == 2)
      SD.remove("test.csv");          // deletes content of SD file when record mode is selected
  }

  while (digitalRead(button1) == 0) // turns on the green led to show press has been recieved
                                    // also functions as error handler incase button is held to long
  {
    digitalWrite(green_led, HIGH);
    digitalWrite(red_led, LOW);
    digitalWrite(white_led,LOW);
  }
  while(digitalRead(hmeButton)==0) //activates if the home button is pressed
  {
    Serial.println("homing");
    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("HOMING SEQUENCE ACTIVATED"); // prints homing message to LCD and turns off red and green LEDs
    digitalWrite(red_led,LOW);
    digitalWrite(green_led,LOW);
    
    for(int a=0;a<=5;a++)             // for loop writes the respective home numbers to the respective arduinos
    { 
      digitalWrite(white_led,LOW); //each time a value is sent the white light will pulse once indicating a transmission
      delay(500);                      
      wireTrans(a+1,homePos[a]);
      digitalWrite(white_led,HIGH);
      delay(500);
    }
    digitalWrite(white_led,LOW);
    digitalWrite(modePin,LOW);
    delay(2000);
  }

  return;
}


/*****************************************************************************************************
  MAIN
 ******************************************************************************************************/

void loop()
{

  if (green == 0 && red == 0)               // main statement telling user they are on the "home" menu
  {
    lcd.setCursor(9, 0); 
    lcd.print ("SELECT OPERATION MODE");
    lcd.setCursor(4,1); 
    lcd.print("RECORD (RED)         RUN (GREEN)");
    digitalWrite(red_led, HIGH);
    digitalWrite(green_led, HIGH);
    digitalWrite(white_led, HIGH);
    delay(500);
      wait_for_one_press();                 // used to detect user input * added many times to improve response time*
    delay(500);
      wait_for_one_press();


    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(white_led, LOW);
    delay(500);
      wait_for_one_press();
      delay(500);
      wait_for_one_press();
    lcd.clear();
    
  }


  /**********************************************************************************************
    CODE FOR RECORD
  ********************************************************************************************/
  if (red == 1)       // sub statement for record mode
                      // user has 10 seconds to confirm the desire to record
                      // after 10 seconds program will default back to "home" menu
  {
    for (int c = 0; c < 20 && red == 1; c++)
    {
      lcd.print("          RECORD MODE SELECTED:                  PRESS AGAIN TO CONFIRM");
      digitalWrite(green_led, LOW);
      digitalWrite(red_led, HIGH);
      delay(125);
      wait_for_one_press();
      delay(125);
      wait_for_one_press();
      digitalWrite(red_led, LOW);
      delay(125);
      wait_for_one_press();
      lcd.clear();
      
    }
  

  }
  while (red == 2)            // record begin
                               // user has confirmed the record program and is in record sequence
                               // program must be terminated by the user
                               // pressing the red record button while in this mode will cause the program to exit and default to the "home" menu
  {
    lcd.setCursor(0,0);
    lcd.print( "PROGRAMMING MODE: MANEUVER ARM"); 
    lcd.setCursor(18,1);
    lcd.print("PRESS RED TO TERMINATE");
    digitalWrite(modePin, HIGH);        // turn on red LED and diasble all motors 
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, HIGH);
    wait_for_one_press();



    sensorReading1 = analogRead(potpin1);             // reads all of the joint encoders
    sensorReading2 = analogRead(potpin2);
    sensorReading3 = analogRead(potpin3);
    sensorReading4 = analogRead(potpin4);
    sensorReading5 = analogRead(potpin5);
    sensorReading6 = analogRead(potpin6);

                                                      // build the data string
    dataString = String(sensorReading1) + "," + String(sensorReading2) + "," + String(sensorReading3) + "," + String(sensorReading4) + "," + String(sensorReading5) + "," + String(sensorReading6); 
                                                      // convert values to CSV  
    saveData();                                       // save to SD card
    delay(10);                                        // delay before next write to SD Card, adjust as required
    Serial.println("writing");
    
  }



  /***********************************************************************************************
    CODE FOR READ
  ***********************************************************************************************/
  if (green == 1)                                 // sub statement for play mode
                                                  // user has 10 seconds to confirm the desire to record
                                                  // after 10 seconds program will default back to "home" menu
  {
    for (int c = 0; c < 20 && green == 1; c++)
    {
      lcd.setCursor(11, 0);
      lcd.print("RUN MODE SELECTED:                    PRESS AGAIN TO CONFIRM");
      digitalWrite(red_led, LOW);
      digitalWrite(green_led, HIGH);
      delay(125);
      wait_for_one_press();
      delay(125);
      wait_for_one_press();
      digitalWrite(green_led, LOW);
      delay(125);
      wait_for_one_press();
      delay(125);
      wait_for_one_press();
    }
    
    lcd.clear();

  }

  if (green == 2)                           // playback begin
                                            // user has confirmed the play program and is now in the run sequence
                                            // this program will terminate automatically when the sd card file has been completely read
  {
    lcd.print("             PROGRAM RUNNING                        STAND CLEAR OF ARM ");
    digitalWrite(modePin, LOW);
    digitalWrite(red_led, LOW);               //turn on Green LED and enable all motors
    digitalWrite(green_led, HIGH);




    char values[60];                        // creates a char array to hold the string because "string" isnt a real part of c
    unsigned short bufferSize = 6;          // how many elements are in the int array "how many angles in a line"
    unsigned short angles[bufferSize];      // declares the int array angles
    unsigned short index = 0;               //creates the int index
    int i;



    sensorData = SD.open("test.csv");                            //open the file for reading:
    if (sensorData)
    {
      Serial.println("test.csv:");                         // confirms the file is available

      while (sensorData.available() != 0)                  // read from the file until there's nothing else in it:
      {
        l_line = sensorData.readStringUntil('\n');    // string "l_line" contains the sensor
                                                      //data up until the new line (\n) character

        if (l_line == "")                             //terminates while loop if blank spaces occurr (possibly delete?)
          break;


        l_line.toCharArray(values, l_line.length());  // takes the contests of l_line and puts it into the char array "values"
        char *p = strtok(values, ",");                //breaks up values into smaller arrays separated by the deliminator ","


        while (p != nullptr && index < bufferSize)      //populates angles with smaller arrays from large string
        {
          angles[index++] = atoi(p);                  // converts char array to int and assigns it to an "angles" value
          p = strtok(NULL, ",");                      // ******dont know what this does
        }

        for (i; i < index; i++)              // prints int angles in the array
        {
          Serial.println(angles[i]);
          wireTrans((i + 1), angles[i]);     // wires the int angles to respective motor controllers
        }

        Serial.println("");
        delay(60);                            // delay used for transmission *modify as needed to achieve accurate playback*
        i = 0;
        index = 0;
      }//end while


      sensorData.close();                              // close the file:
      lcd.clear();

    }
    else                                                  // if the file didn't open, print an error:
      lcd.clear();
    delay(50);
    lcd.print("error opening test.txt");




    //delay(10000); // delay between repeat of entire sd memory
    green = 0;
  }
  green = 0;      // reset all button counts and clear LCD
  red = 0;
  lcd.clear();
  digitalWrite(modePin,LOW); //enable all motors
}
