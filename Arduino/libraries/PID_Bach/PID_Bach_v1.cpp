/**********************************************************************************************
 * Arduino PID Library - Version 1.1.1
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 * edited by Ida Stoustrup for use with an iodine frequency reference
 *
 * This Library is licensed under a GPLv3 License
 **********************************************************************************************/

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <PID_Bach_v1.h>

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(uint16_t* Input, uint16_t* InputRef, uint16_t* OutputSmall, uint16_t* OutputBig, uint16_t* Setpoint, uint16_t* VerboseNum,
         int* DirecAddr, bool* AutoAddr, bool* VerbAddr, float* HillAddr)
{
    myOutputS = OutputSmall;
    myOutputB = OutputBig;
    myInput = Input;
    myInputRef = InputRef;
    mySetpoint = Setpoint;
    myVerboseNum = VerboseNum;
    DirecPoin = DirecAddr;
    AutoPoin = AutoAddr;
    HillPoin = HillAddr;
    VP = VerbAddr;
    
    //controllerDirection = DIRECT;
    lastAuto = false;
    outputSum = 0;
    
    ampcon = 1;

    PID::SetOutputLimits(0, 3000);
    offcounter = 0;
    kp = 0.0001;
    ki = 0;
    kd = 0;
    
    BSfac = 43.5;
    
    //PID::SetControllerDirection(DIRECT);
    
}


/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/
bool PID::Compute()
{
    if(!*AutoPoin) {
        lastAuto = false;
        *DirecPoin = 2;
        *myOutputS = 2000;
        return false;
    }
if (*AutoPoin == !lastAuto) PID::Initialize();
   
  lastAuto = true;
    
  float Kp, Ki, Kd;
    
  Kp = ampcon * kp;
  Ki = ampcon * ki;
  Kd = ampcon * kd;
   
  /*Compute all the working error variables*/
  uint16_t input = *myInput;            // can't overflow because read res is 16 bit too
  int32_t error = *mySetpoint - input;  // shouldn't overflow because the error would need to
                                        // be over 32768 or under -32768
  int32_t dInput = (input - lastInput); // shouldn't overflow for same reason as error
    
  outputSum += Ki * error;
    
  if(outputSum > outMax) outputSum= outMax;
  else if(outputSum < outMin) outputSum= outMin;

  int16_t output = outputSum;   // this needs 12 bit unsigned since that's the output range
                                        // I made it signed since then going a little too low will make
                                        // it negative and thus bring it to the min instead of it over-
                                        // flowing and going to max
  int16_t outputS = 2000;
    

  /*Compute Rest of PID Output*/
    
  float PandD = Kp * error - Kd * dInput;

  output += PandD;
  outputS += int16_t((outputSum + PandD - output)*BSfac + 0.5);

  if(output > outMax)
  {
      output = outMax;
      //*DirecPoin = 2; // Backwards
      //*AutoPoin = false;
  }
  else if(output < outMin)
  {
      output = outMin;
      //*DirecPoin = 4; // Forwards
      //*AutoPoin = false;
  }
    
  if(outputS > outMax)
  {
    outputS = outMax;
  }
  else if(outputS < outMin)
  {
      outputS = outMin;
  }
    
  *myOutputS = -outputS;
  *myOutputB = output;
    

  /*Remember some variables for next time*/
  lastInput = input;
    return true;
   
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID::SetOutputLimits(int Min, int Max)
{
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;

   if(*AutoPoin)
   {
	   if(*myOutputB > outMax) *myOutputB = outMax;
	   else if(*myOutputB < outMin) *myOutputB = outMin;

	   if(outputSum > outMax) outputSum= outMax;
	   else if(outputSum < outMin) outputSum= outMin;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void PID::SetMode()
{
    *AutoPoin = !*AutoPoin;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void PID::Initialize()
{
//   outputSum = *myOutput;
//   if(outputSum > outMax) outputSum = outMax;
//   else if(outputSum < outMin) outputSum = outMin;
   outputSum = *myOutputB;
   lastInput = *myInput;
}


