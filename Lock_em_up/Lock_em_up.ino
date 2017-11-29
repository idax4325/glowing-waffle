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
#define PIN_OUTBIG A21
#define PIN_OUTSMALL A22

const int led = LED_BUILTIN;

char inChar;

bool sendinput, sendinputref, sendoutput;
int sendcount = 0;

bool OnResonance = false;
bool PIDAuto = true;
bool verbosemode = false;
bool scanning = false;

int inputcount = 0;

uint16_t Input, InputRef, VerboseNum;
uint16_t OutBig = 0, OutSmall = 0, Setpoint = 27000;

float HillHeight = 700; 

ResFind myResFind(&Input, &InputRef, &OutBig, &Setpoint, &VerboseNum, &PIDAuto, &verbosemode, &HillHeight);

// Choose the initial values for the PID constants
float Kp_small=0, Ki_small=0, Kd_small=0;
float Kp_big=0, Ki_big=0, Kd_big=0;

PID smallPID(&Input, &InputRef, &OutSmall, &Setpoint, &VerboseNum, &myResFind.Direction, &PIDAuto, &verbosemode, &HillHeight);

PID bigPID(&Input, &InputRef, &OutBig, &Setpoint, &VerboseNum, &myResFind.Direction, &PIDAuto, &verbosemode, &HillHeight);


int OutputMin = 150, OutputMax = 4045;

void setup() {

  Serial.begin(9600);  // The int shouldn't matter since the teensy is always at USB speed

  pinMode(led, OUTPUT);
  pinMode(PIN_INPUT_PDH, INPUT);
  pinMode(PIN_INPUT_REF, INPUT);
  pinMode(PIN_OUTSMALL, OUTPUT);
  pinMode(PIN_OUTBIG, OUTPUT);
  analogWriteResolution(12);
  analogReadResolution(16);
  
// Set the output limits

   myResFind.SetLimits(OutputMin,OutputMax); 
   smallPID.SetOutputLimits(OutputMin,OutputMax);
   bigPID.SetOutputLimits(OutputMin,OutputMax);

// Set the PID constants

  smallPID.kp = Kp_small;   
  smallPID.ki = Ki_small;
  smallPID.kd = Kd_small;

  bigPID.kp = Kp_big;   
  bigPID.ki = Ki_big;
  bigPID.kd = Kd_big;

// Set the direction of PID

  smallPID.PIDforward = false;
  bigPID.PIDforward = false;

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
        if(!OnResonance) 
        myResFind.Running();
        else
        smallPID.SetMode();
        bigPID.SetMode();
        
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
      case 'E': {   // toggle whether input is sent
        Serial.write('R');
        Serial.write('E');
        if(sendinputref) {
          sendinputref = false;
        }
        else {
          sendinputref = true;
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
        Serial.write('T');
        Serial.write('V');
        if(verbosemode)
          verbosemode = false;
        else
          verbosemode = true;
        serial_write_i(VerboseNum);
          
        break;
      }
      case 'P': {   // change value of Kp 
        char inchar2 = Serial.read();
        switch(inchar2) {
          case'S': {
            float newvalue = serial_read_f();
            smallPID.kp = newvalue;  

            Serial.write('R');
            Serial.write('P');
            serial_write_f(smallPID.kp);
            break;
          }
          case'B': {
            float newvalue = serial_read_f();
            bigPID.kp = newvalue;  

            Serial.write('R');
            Serial.write('P');
            serial_write_f(bigPID.kp);
            break;
          }  
        break;
        }
      }
      case 'I': {   // change value of Ki
        char inchar2 = Serial.read();
        switch(inchar2) {
          case'S': {
            float newvalue = serial_read_f();
            smallPID.ki = newvalue;  

            Serial.write('R');
            Serial.write('I');
            serial_write_f(smallPID.ki);
            break;
          }
          case'B': {
            float newvalue = serial_read_f();
            bigPID.ki = newvalue;  

            Serial.write('R');
            Serial.write('I');
            serial_write_f(bigPID.ki);
            break;
          }  
        break;
        }
      }
      case'D': {    // change value of Kd
        char inchar2 = Serial.read();
        switch(inchar2) {
          case'S': {
            float newvalue = serial_read_f();
            smallPID.kd = newvalue;  

            Serial.write('R');
            Serial.write('D');
            serial_write_f(smallPID.kd);
            break;
          }
          case'B': {
            float newvalue = serial_read_f();
            bigPID.kd = newvalue;  

            Serial.write('R');
            Serial.write('D');
            serial_write_f(bigPID.kd);
            break;
          }  
        break;
        }
      }
      case'Z': {
        char inchar2 = Serial.read();
        switch(inchar2) {
          case'S': {            // set SP sent by python gui and send it as well to ensure correct transfer
            myResFind.allowSPchange = false;  // ResFind shouldn't change the Setpoint if we've chosen a value
            uint16_t newSP = serial_read_i();
            Setpoint = newSP;
            Serial.write('R');
            Serial.write('Z');
            serial_write_i(Setpoint);
            break;
          }
          case'G': {            // give SP to python gui
            Serial.write('T');
            Serial.write('Z');
            serial_write_i(Setpoint);
            break;
          break;
          }
        }
      }
      case'C': {
        char inchar2 = Serial.read();
        switch(inchar2) {
          case'S': {
            
            uint16_t newRS = serial_read_i();
            myResFind.RampSlow = newRS;
            
            scanning = true;

            Serial.write('R');
            Serial.write('C');
            break;
          }
          case'T': {
            scanning = false;
            Serial.write('R');
            Serial.write('C');
            break;
          }
        }
        
        break;
      }
    }
  }

  Input = analogRead(PIN_INPUT_PDH);
  InputRef = analogRead(PIN_INPUT_REF);

//   if(InputRef < HillHeight * 0.5)      // TEMPORARILY DISABLED FOR TESTING PURPOSES
//       offcounter++;
//   else
//       offcounter = 0;
//
//   if(offcounter > 100) 
//   {
//       offcounter = 0;
//       myResFind.Direction = 2;
//       PIDAuto = false;
//   }

  if(scanning) {

    myResFind.Ramp();
    
  }

  else {

  if(!OnResonance) {
    
  digitalWrite(led, LOW);
  OnResonance = myResFind.TakeMeThere(); // Take me to the resonance!
  
  }
  
  else {

  digitalWrite(led, HIGH);
  smallPID.Compute(); // Run the PID to find the correct output
  bigPID.Compute();

  }

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
      Serial.send_now(); // adding this makes sure that data packages aren't divided on two sends, but could make transfer slower
    }
  
  }

    if(sendinputref) {
    
    sendcount++;
    if(sendcount == 250) {
      Serial.write('T');
      Serial.write('U');
      uint16_t timenow = (uint16_t) millis();
      serial_write_i(timenow);
      serial_write_i(InputRef);
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
      serial_write_i(OutSmall);
      sendcount = 0;
      Serial.send_now(); // adding this makes sure that data packages aren't divided on two sends
    }
  
  }


  // Write the output to the chosen output pin
  analogWrite(PIN_OUTSMALL, OutSmall);
  analogWrite(PIN_OUTBIG, OutBig);

}

union intUnion {  
    uint16_t i;  
    byte bytes[2];  
};

union floatUnion {  
    float f;  
    byte bytes[4];  
};


int serial_read_i(){
  intUnion iU;
  iU.bytes[1] = Serial.read(); // msb
  iU.bytes[0] = Serial.read(); // lsb
  return iU.i;
}

void serial_write_i(int data){
  intUnion iU;
  iU.i = data;
  Serial.write(iU.bytes[1]); // msb
  Serial.write(iU.bytes[0]); // lsb
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
