/********************************************************************************
 * Adam and Idas amaze-balls, awe-inspiring, acclaimed script for cavity-locking
 * This script is designed to hunt down resonances and lock em up using PID.
 ********************************************************************************/

#include <ResFind.h>
#include <PID_Bach_v1.h>
#include <SPI.h>
//#include <PID_AutoTune_v0.h>

#define PIN_INPUT_PDH A1
#define PIN_INPUT_REF A4
#define PIN_OUTPUT A21

const int led = LED_BUILTIN;

char inChar;

bool sendinput, sendoutput;
int sendcount = 0;

bool OnResonance = false;
bool PIDAuto = true;
bool verbosemode = false;

int inputcount = 0;

uint16_t Input, InputRef, VerboseNum;
uint16_t Output = 0, Setpoint = 27000;

float HillHeight = 1000; 

ResFind myResFind(&Input, &InputRef, &Output, &Setpoint, &VerboseNum, &PIDAuto, &verbosemode, &HillHeight);

// Choose the initial values for the PID constants
int Kp=6, Ki=3, Kd=0;

PID myPID(&Input, &InputRef, &Output, &Setpoint, &VerboseNum, &myResFind.Direction, &PIDAuto, &verbosemode, &HillHeight);

int OutputMin = 150, OutputMax = 4045;

void setup() {

  Serial.begin(9600);  // The int shouldn't matter since the teensy is always at USB speed

  pinMode(led, OUTPUT);
  pinMode(PIN_INPUT_PDH, INPUT);
  pinMode(PIN_INPUT_REF, INPUT);
  pinMode(PIN_OUTPUT, OUTPUT);
  analogWriteResolution(12);
  analogReadResolution(16);
  
  Input = analogRead(PIN_INPUT_PDH);


// Set the output limits

   myResFind.SetLimits(OutputMin,OutputMax); 
   myPID.SetOutputLimits(OutputMin,OutputMax);

// Set the PID constants

  myPID.kp = Kp;    // the sign isn't checked here because it's set just after and we don't wanna 
  myPID.ki = Ki;
  myPID.kd = Kd;

// Set the direction of PID

  myPID.PIDforward = true;

// Put the PID in automatic mode (as opposed to manual where it's turned off)

//   myPID.SetMode(AUTOMATIC); Value already set.
}

void loop() {

  if (Serial.available()) {
    inChar = Serial.read();
    switch(inChar) {
      case 'S': {
        Serial.write('R');
        Serial.write('S');
        if(!OnResonance) // add option to pause things. the run functions in PID and ResFind should have an optional input and otherwise switch current setting
        myResFind.Running();
        else
        myPID.SetMode();
        
        break;
      }
      case 'N': {   // toggle whether input is sent
        Serial.write('R');
        Serial.write('N');
        if(sendinput) {
          sendinput = false;
        }
        else {
          sendinput = true;
        }
        break;
      }
      case 'O': {   // toggle whether output is sent
        Serial.write('R');
        Serial.write('O');
        if(sendoutput)
          sendoutput = false;
        else
          sendoutput = true;
        break;
      }
      case 'V': { // toggle verbose mode
        Serial.write('R');
        Serial.write('V');
        if(verbosemode)
          verbosemode = false;
        else
          verbosemode = true;
        VerboseNum = Setpoint;
        serial_write_i(VerboseNum);
          
        break;
      }
      case 'P': {   // change value of Kp 

        float newvalue = serial_read_f();
        
        myPID.kp = newvalue;  

        Serial.write('R');
        Serial.write('P');
        serial_write_f(myPID.kp);
          
        break;
        
      }
      case 'I': {   // change value of Ki
        
        float newvalue = serial_read_f();
               
        myPID.ki = newvalue;  

        Serial.write('R');
        Serial.write('I');
        serial_write_f(myPID.ki);
          
        break;
      }
      case'D': {    // change value of Kd
        
        float newvalue = serial_read_f();
               
        myPID.kd = newvalue;  

        Serial.write('R');
        Serial.write('D');
        serial_write_f(myPID.kd);
          
        break;
      }
    }
  }

  Input = analogRead(PIN_INPUT_PDH);
  InputRef = analogRead(PIN_INPUT_REF);

  if(!OnResonance) {
    
  digitalWrite(led, LOW);
  OnResonance = myResFind.TakeMeThere(); // Take me to the resonance!
  
  }
  
  else {

  digitalWrite(led, HIGH);
  OnResonance = myPID.Compute(); // Run the PID to find the correct output

  }
  

  if(sendinput) {
    
    sendcount++;
    if(sendcount == 250) {
      Serial.write('T');
      Serial.write('U');
      uint16_t timenow = (uint16_t) millis();
      serial_write_i(timenow);
      serial_write_i(Input);
      sendcount = 0;
      Serial.send_now(); // adding this makes sure that data packages aren't divided on two sends
    }
  
  }

  if(sendoutput) {
    
    sendcount++;
    if(sendcount == 250) {
      Serial.write('T');
      Serial.write('U');
      uint16_t timenow = (uint16_t) millis();
      serial_write_i(timenow);
      serial_write_i(Output);
      sendcount = 0;
      Serial.send_now(); // adding this makes sure that data packages aren't divided on two sends
    }
  
  }


  // Write the output to the chosen output pin
  analogWrite(PIN_OUTPUT, Output);

}

union intUnion {  
    int i;  
    byte bytes[2];  
};

union floatUnion {  
    float f;  
    byte bytes[4];  
};

void serial_write_i(int data){
  intUnion iU;
  iU.i = data;
  Serial.write(iU.bytes[0]); // msb
  Serial.write(iU.bytes[1]); // lsb
}

float serial_read_f(){    // if used with a device which is in big-endian this order should be reversed
  floatUnion fU;
  fU.bytes[3] = Serial.read(); // msb1 (the teensy is little-endian and we're sending big-endian from python, thus order is in reverse)
  fU.bytes[2] = Serial.read(); // b2
  fU.bytes[1] = Serial.read(); // b3
  fU.bytes[0] = Serial.read(); // lsb4
  return fU.f;
}

void serial_write_f(float data){
  floatUnion fU;
  fU.f = data;
  Serial.write(fU.bytes[3]); // msb1
  Serial.write(fU.bytes[2]); // b2
  Serial.write(fU.bytes[1]); // b3
  Serial.write(fU.bytes[0]); // lsb4
}
