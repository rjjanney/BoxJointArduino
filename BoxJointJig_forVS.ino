

// Rotary Encoder libraries
#include <ClickEncoder.h>
#include <TimerOne.h>

// Motor Control libraries
#include <AFMotor.h> // to control the stepper

 // button input auto debounce oop
#include <Button.h> 

// Adafruit Nokia 5110 control libraries and gfx library
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// Software SPI (slower updates, more flexible pin options):
// pin 29 - Serial clock out (SCLK)
// pin 27 - Serial data out (DIN)
// pin 25 - Data/Command select (D/C)
// pin 23 - LCD chip select (CS)
// pin 21 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(29, 27, 25, 23, 21);

// setup strings
int kerf[4] = {0, 0, 0, 0};
int dist[4] = {2, 2, 2, 2};
int test;
int _b;
int activeDigit = 0;

// pwm capable pin for lcd backlight dimming
int ledPin = 2;

// Strings for simple UI
String line1 = "BOX JOINT CTL\n\n";
String line2 = " KERF .";
String line3 = " DIST .";
String line4 = "\nCUT NOW";
String inchesOrMilimeters = "\"\n"; // keeps track of unit scale

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
AF_Stepper motor(200, 2);

// part of setup of rotary encoder with button
ClickEncoder *encoder;
int16_t last, value;

void timerIsr() {
  encoder->service();
}

void setup() {
  encoder = new ClickEncoder(30, 31, 32);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = -1;
  
    
  display.begin();
  
  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(60);
  
  // set led brightness via pwm
  analogWrite(ledPin, 128);

 // display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.display();

}

void loop() {

  // text display tests
  // at text size 1 you have 80 characters
  // 6 lines of 13 chars 
  display.clearDisplay();   // clears the screen and buffer 

//////////////////////////////////////////////////////////
// ENCODER CHECKING/READING ROUTINE //////////////////////
  value += encoder->getValue();
  
  if (value != last) {
    if (value < 0){value += 1000;}
    last = floor(abs(value/2.8));
    // line2 = "Hi " + String(last%10) + "\n";
  }
  
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    #define VERBOSECASE(label);
    switch (b) {
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Released)
      case ClickEncoder::DoubleClicked:
        // change inches to mm, in string and in calculations
        // set up a units flag variable
		  if (inchesOrMilimeters == "\"\n") {
			  inchesOrMilimeters = "mm\n";
		  }
		  else {
			  inchesOrMilimeters = "\"\n";
		  }
        break;
      case ClickEncoder::Clicked:
        // move on to next digit or next field
       activeDigit ++;
	   if (activeDigit > 7) { activeDigit = 0; }
        break;
      case ClickEncoder::Held:
        // move decimal point to left of cursor
        _b += 100;
        break;
    }
  }
  //////////////////////////////////////////////////////////////
  
  
  //display.setTextColor(WHITE, BLACK); // 'inverted' text
  display.print(line1 + line2);
  for (int j = 0; j < 4; ++j) {
	  if (j == activeDigit) {
		  display.setTextColor(WHITE, BLACK); // 'inverted' text
		  display.print(kerf[j]);
		  display.setTextColor(BLACK); // back to normal text
	  }
	  else {
		  display.print(kerf[j]);
	  }
  }
  display.print(inchesOrMilimeters);
  display.print(line3);
  // TO SET UP FOR "ACTIVE" character being edited, array 0-7,
  // corresponding to these print statements
  for (int j = 4; j < 8; ++j) {
	  if (j == activeDigit) {
		  display.setTextColor(WHITE, BLACK); // 'inverted' text
		  display.print(dist[j-4]);
		  display.setTextColor(BLACK); // back to normal text
	  }
	  else {
		  display.print(dist[j - 4]);
	  }
  }
  display.print(inchesOrMilimeters);
  display.print(line4);
    
  display.display();
  

}

