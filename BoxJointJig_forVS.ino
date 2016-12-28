

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
int kerfDist[8] = {1, 0, 0, 0, 3, 0, 0, 0};
int activeDigit = 0;
const double STEPDISTANCE = .0003125;
bool loopFlag = false;
bool readyToCut = false;
bool stopSign = true;
bool escapeButton = false;

// setup buttons

Button forward = Button(39, BUTTON_PULLUP_INTERNAL, true, 30);
Button backward = Button(41, BUTTON_PULLUP_INTERNAL, true, 30);
Button action = Button(43, BUTTON_PULLUP_INTERNAL, true, 30);



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
AF_Stepper motor(100, 2);

// part of setup of rotary encoder with button
ClickEncoder *encoder;
int16_t last, value;

void goForward(Button& b) {
	while (forward.isPressed()) {
		motor.step(100, BACKWARD, DOUBLE);
	}
	return;
}

void goBackward(Button& b) {
	escapeButton = true;
	while (backward.isPressed()) {
		motor.step(100, FORWARD, DOUBLE);
	}
}

void moveSled(int numberOfSteps) {
	motor.step(numberOfSteps, BACKWARD, DOUBLE);
	return;
}

void goNextCut(Button& b) {
	stopSign = false;
}

void delayPrint(String message) {
	display.clearDisplay();
	display.print(message);
	display.display();
	delay(100);
	return;
}

int stepsToGo(double distance) {
	return ceil(distance / STEPDISTANCE);
}



void timerIsr() {
  encoder->service();
}

void cutRoutine(int valueArray[], String units) {
	display.clearDisplay();
	double kerfCut = valueArray[0] * 0.1 + valueArray[1] * 0.01 + valueArray[2] * 0.001 + valueArray[3] * 0.0001;
	double distCut = valueArray[4] * 0.1 + valueArray[5] * 0.01 + valueArray[6] * 0.001 + valueArray[7] * 0.0001;
	if (kerfCut > distCut) {
		delayPrint("DISTANCE LESS\nTHAN KERF");
		readyToCut = false;
		return;
	}
	else if (kerfCut == 0.0) {
		delayPrint("KERF = 0?!?!?");
		readyToCut = false;
		return;
	}
	else {
		// here's where all the motor control and calculations go
		// figure out waypoints as seq of array values (motor steps)
		while (!escapeButton) {
			double toGo = distCut - kerfCut;
			while (toGo > 0) {
				if (toGo > kerfCut) {
					delayPrint("WAIT - MOVING SLED kerf dist");
					moveSled(stepsToGo(kerfCut));
					String stepsReturn = String(stepsToGo(kerfCut));
					delayPrint(stepsReturn);
					toGo -= kerfCut;
					stopSign = true;
					while (stopSign) {
						backward.process();
						action.process();
					}
				}
				else {
					delayPrint("WAIT - MOVING SLED the rest");
					moveSled(stepsToGo(toGo));
					delayPrint("CUT NOW!");
					stopSign = true;
					while (stopSign) { 
						backward.process();
						action.process();
					}
					toGo = 0;
				}
			}
			delayPrint("WAIT - MOVING SLED cut distance plus kerf");
			moveSled(stepsToGo(distCut + kerfCut));
			delayPrint("CUT NOW!");
			stopSign = true;
			while (stopSign) {
				backward.process();
				action.process();
			}
		}
		
		readyToCut = false;
		return;
	}
}


void setup() {
  encoder = new ClickEncoder(30, 31, 32);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = -1;

  // motor setup
  motor.setSpeed(100);


  // Assign callback function
  forward.pressHandler(goForward);
  backward.pressHandler(goBackward);
  action.pressHandler(goNextCut);
  
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

	// update the buttons' internals
	forward.process();
	backward.process();
	escapeButton = false;
	action.process();

	// text display tests
	// at text size 1 you have 80 characters
	// 6 lines of 13 chars 
	display.clearDisplay();   // clears the screen and buffer 

  //////////////////////////////////////////////////////////
  // ENCODER CHECKING/READING ROUTINE //////////////////////
	value += encoder->getValue();

	if (value != last) {
		if (value < 0) { value += 1000; }
		last = floor(abs(value / 2.8));
		// line2 = "Hi " + String(last%10) + "\n";
		kerfDist[activeDigit] = last % 10;
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
			activeDigit++;
			if (activeDigit > 7) {
				activeDigit = 0;
				loopFlag = true;
			}
			if (loopFlag) {		// If this is not first time through, 
								// keep values when activating them
				value = floor(kerfDist[activeDigit] * 3);
			}
			else {				// First time through, set initial value to 0 for each digit

				value = 0;
			}
			break;
		case ClickEncoder::Held:
			// Ready to start cutting
			readyToCut = true;
			break;
		}
	}
		//////////////////////////////////////////////////////////////


		//------------------------------------------------------------------//
		//                    USER INTERFACE                                //
		display.print(line1 + line2);
		for (int j = 0; j < 4; ++j) {
			if (j == activeDigit) {
				display.setTextColor(WHITE, BLACK); // 'inverted' text
				display.print(kerfDist[j]);
				display.setTextColor(BLACK); // back to normal text
			}
			else {
				display.print(kerfDist[j]);
			}
		}
		display.print(inchesOrMilimeters);
		display.print(line3);
		// TO SET UP FOR "ACTIVE" character being edited, array 0-7,
		// corresponding to these print statements
		for (int j = 4; j < 8; ++j) {
			if (j == activeDigit) {
				display.setTextColor(WHITE, BLACK); // 'inverted' text
				display.print(kerfDist[j]);
				display.setTextColor(BLACK); // back to normal text
			}
			else {
				display.print(kerfDist[j]);
			}
		}
		display.print(inchesOrMilimeters);
		display.print(line4);

		display.display();

		if (readyToCut) {
			cutRoutine(kerfDist, inchesOrMilimeters);
		}

	}


