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
PID::PID(uint16_t* Input, uint16_t* InputRef, uint16_t* Output, uint16_t* Setpoint, uint16_t* VerboseNum,
         int* DirecAddr, bool* AutoAddr, bool* VerbAddr, float* HillAddr)
{
    myOutput = Output;
    myInput = Input;
    myInputRef = InputRef;
    mySetpoint = Setpoint;
    myVerboseNum = VerboseNum;
    DirecPoin = DirecAddr;
    AutoPoin = AutoAddr;
    HillPoin = HillAddr;
    VP = VerbAddr;
    
    controllerDirection = DIRECT;
    lastAuto = false;

    sf = 12;
    PID::SetOutputLimits(0, 3000);
    offcounter = 0;
    kp = 1;
    ki = 0;
    kd = 0;
    
    PID::SetControllerDirection(DIRECT);
    
}

/*Constructor (...)*********************************************************
 *    To allow backwards compatability for v1.1, or for people that just want
 *    to use Proportional on Error without explicitly saying so
 ***************************************************************************/

//PID::PID(double* Input, double* Output, double* Setpoint, int* DirecAddr,
//        double Kp, double Ki, double Kd, int ControllerDirection)
//    :PID::PID(Input, Output, Setpoint, DirecAddr, Kp, Ki, Kd, P_ON_E, ControllerDirection)
//{
//
//}


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
        return false;
    }
if (*AutoPoin == !lastAuto) PID::Initialize();
//   if(*myInputRef > *HillPoin * 0.6)      // TEMPORARILY DISABLED FOR TESTING PURPOSES
//       offcounter++;
//   if(offcounter > 20)  // SHOULD BE 20 times in a row
//   {
//       offcounter = 0;
//       *DirecPoin = 2;
//       *AutoPoin = false;
//       lastAuto = *AutoPoin;
//       //*myOutput = 2047; // seems unnecessary because of the way ResFind ramps
//       return false;
//   }
   
  lastAuto = true;
   
  /*Compute all the working error variables*/
  uint16_t input = *myInput;
  int16_t error = *mySetpoint - input;
  int16_t dInput = (input - lastInput);
  
  int32_t tempOS = ki * error;  // we need a 4 byte int
    
  tempOS = tempOS >> sf;
  
  outputSum += tempOS;

  if(outputSum > outMax) outputSum= outMax;
  else if(outputSum < outMin) outputSum= outMin;

  uint16_t output;
    

  /*Compute Rest of PID Output*/
    
  int32_t tempOP = kp * error - kd * dInput;
    
  tempOP = tempOP >> sf;
    
  output = outputSum + tempOP;
    
  output += lastResFindOutput;
    
  if(output > outMax)
  {
      output = outMax; // 80 % of outmax
      *DirecPoin = 2; // Backwards
      //*AutoPoin = false;
  }
  else if(output < outMin)
  {
      output = outMin; // 20 % of outmax
      *DirecPoin = 4; // Forwards
      //*AutoPoin = false;
  }
    
    *myOutput = output;

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
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;

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
   outputSum = 0;
   lastResFindOutput = *myOutput;
   lastInput = *myInput;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int Direction)
{
   if(Direction !=controllerDirection) // deleted requirement for *AutoPoin = true
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/

int PID::GetMode(){ return  *AutoPoin ? AUTOMATIC : MANUAL;}
int PID::GetDirection(){ return controllerDirection;}

