

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
int kerf[] = {0, 0, 0, 0};
int dist[] = {0, 0, 0, 0};
int test;
int _b = 0;

// pwm capable pin for lcd backlight dimming
int ledPin = 2;

// Strings for simple UI
String line1 = "Box Joint Ctl\n";
String line2 = "Kerf .\"\n";
String line3 = "Dist .\"\n";
String line4 = "\nCUT NOW";

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


// setup joystick button
//  pinMode(SW_pin, INPUT);
//  digitalWrite(SW_pin, HIGH);
//  
}

void loop() {

  // text display tests
  // at text size 1 you have 80 characters
  // 6 lines of 13 chars 
  display.clearDisplay();   // clears the screen and buffer 


  value += encoder->getValue();
  
  if (value != last) {
    if (value < 0){value += 1000;}
    last = floor(abs(value/2.8));
    line2 = "Hi " + String(last%10) + "\n";
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
        _b += 1;
        break;
      case ClickEncoder::Clicked:
        // move on to next digit or next field
        _b += 10;
        break;
      case ClickEncoder::Held:
        // move decimal point to left of cursor
        _b += 100;
        break;
    }
  }
  display.print(String(_b) + "\n" + line2 + line3 + line4);
    
  display.display();
  

}

