// This file is part of Arduino ST4.
//
// Arduino ST4 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Arduino ST4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Arduino ST4.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright Kevin Ferrare 2014

#include <Stepper.h>

const int fastPlusPin = 7;
const int fastMinusPin = 5;
const int togglePin = 6;
const int potPin = A5;

int lastFastPlusState = HIGH;
int lastFastMinusState = HIGH;
int lastToggleState = HIGH;

const int stepsPerRevolution = 48;

// Sidereal Day in minutes.
const float siderealDay = 1436.07;
float siderealRate = 66480 / siderealDay;

int stepperDir = 1;
int stepperToggle = 1;
float slewMult = 6;
float trackMult = 0.5;

// Initialize the stepper library on pins 8 through 11:
Stepper stepper(stepsPerRevolution, 8, 9, 10, 11);

/*
 * Led indicates whether the software requested connection, it is shut off when the software disconnects
 */
const int pinLED = 2;

/**
 * An axis has a pin per direction.
 * Both pins cannot be up at the same time.
 */
class Axis {
private:
  int plusPin;
  int minusPin;
public:
  Axis(int plusPin, int minusPin) : 
  plusPin(plusPin), minusPin(minusPin) {
  }
  void setupPins(){
    stepperDir = 1;
    stepper.setSpeed(siderealRate);

    pinMode(fastPlusPin, INPUT_PULLUP);
    pinMode(fastMinusPin, INPUT_PULLUP);
    pinMode(togglePin, INPUT_PULLUP);
  }
  void plus(){
    stepperDir = 1;
    stepper.setSpeed(trackMult * siderealRate);
  }
  void minus(){
    stepperDir = -1;
    stepper.setSpeed(trackMult * siderealRate);
  }
  void reset(){
    stepper.setSpeed(siderealRate);
  }
};

class Axis rightAscension(
2,//RA+ pin
5);//RA- pin
class Axis declination(
3,//DEC+ pin
4);//DEC- pin

void setup()
{
  rightAscension.setupPins();
  //declination.setupPins();
  pinMode(pinLED, OUTPUT);
  //57.6k, 8 data bits, no parity, one stop bit.
  Serial.begin(57600, SERIAL_8N1);
  //Wait for serial port to connect. Needed for Leonardo only
  while (!Serial);
  Serial.println("INITIALIZED#");
}


void loop()
{
  stepper.step(stepperToggle * stepperDir);
  
  if (Serial.available() > 0) {
    //Received something
    String opcode = Serial.readStringUntil('#');
    boolean validOpcode=true;
    //Parse opcode
    if(opcode=="CONNECT"){
      digitalWrite(pinLED, HIGH);
      resetPins();
    }
    else if (opcode=="DISCONNECT"){
      digitalWrite(pinLED, LOW);
      resetPins();
    }
    else if(opcode=="RA0"){
      rightAscension.reset();
    }
    else if(opcode=="RA+"){
      rightAscension.plus();
    }
    else if(opcode=="RA-"){
      rightAscension.minus();
    }
    else if(opcode=="DEC0"){
      // Not implemented.
      //declination.reset();
    }
    else if(opcode=="DEC+"){
      // Not implemented.
      //declination.plus();
    }
    else if(opcode=="DEC-"){
      // Not implemented.
      //declination.minus();
    }
    else{
      validOpcode=false;
    }
    if(validOpcode){
      //Acknowledge valid command
      Serial.println("OK#");
    }
  }

  int fastPlusState = digitalRead(fastPlusPin);
  int fastMinusState = digitalRead(fastMinusPin);
  int toggleState = digitalRead(togglePin);
  if (fastPlusState == LOW){
    stepperDir = 1;
    stepper.setSpeed(slewMult * siderealRate);
  }
  else if (fastMinusState == LOW){
    stepperDir = -1;
    stepper.setSpeed(slewMult * siderealRate);
  }
  else if ((toggleState == LOW) && (lastToggleState == HIGH)){
    stepperToggle = (stepperToggle == 0) ? 1 : 0;
  }
  else if (((fastPlusState == HIGH) && (lastFastPlusState == LOW)) || 
          (((fastMinusState == HIGH) && (lastFastMinusState == LOW)))){
    stepperDir = 1;
    stepper.setSpeed(siderealRate);
  }
  lastFastPlusState = fastPlusState;
  lastFastMinusState = fastMinusState;
  lastToggleState = toggleState;

  int potVal = analogRead(potPin);
  siderealRate = map(potVal, 0, 1023, 66480 - 2000, 66480 + 2000) / siderealDay;
}

void resetPins(){
  rightAscension.reset();
  declination.reset();
}
