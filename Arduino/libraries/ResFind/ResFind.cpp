#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ResFind.h>
#include <cmath>


ResFind::ResFind(uint16_t* Input, uint16_t* InputRef, uint16_t* Output, uint16_t* Setpoint, uint16_t* VerboseNum, bool* AutoAddr, bool* VerbAddr, int* HillAddr)
{
    myInput = Input;        // Pass pointer to the input and output to the ResFind
    myInputRef = InputRef;
    myOutput = Output;
    mySetpoint = Setpoint;
    myVerboseNum = VerboseNum;
    AutoPoin = AutoAddr;
    HillPoin = HillAddr;
    VP = VerbAddr;
    
    lasttimeoffset = 28000;
    
    OutputMax = 2500; // Set default output max and min
    OutputMin = 0;
    
    Direction = FORWARDS;   // Set default direction
    
    running = false;        // Set this boolean to true so it can be changed to false with the pause
                            // functions and thus stop the search if necessary
    close = false;          // A boolean used to check if we're close to resonance
    
    //iWait = 0;              // parameter that makes it possible to wait a bit with declaring resonance
                            // after seeing PDH start. Might not be useful
    
    RampSlow = 100;
    outpu = 0, q = 0;
    
    ResFind::Initialize();
    
}



void ResFind::Running(bool run)
{
	running = run; // An input of 1 means yes TakeMeThere should run, and 0 the opposite.
}

 
bool ResFind::TakeMeThere()
{
    if(running)
    {
        if(Direction == 2)
        {
            ResFind::Initialize();
            Direction = BACKWARDS;
        }
        else if(Direction == 4)
        {
            ResFind::Initialize();
            Direction = FORWARDS;
        }
        
        input = (float)*myInput;        // Creates a variable just for this iteration of this function with
        inputRef = (float)*myInputRef;  // the value of the input. The class knows the address of the input
                                        // (stored in the pointer myInput) so it can just look up the value.
        i++;
        
        offset += input;        // add a bit of the input to the offset
        P_ar[i] = input;
        roffset += inputRef;
        r_ar[i] = inputRef;
        
        if(inputRef > roffset + rstd * 50) {
            offset = 0;
            roffset = 0;
            i = 0;
        }
        
        if(i % OffStepsize == 0 && i!= 0)            // is the offset done collecting inputs?
        {
            float tempOffSS = OffStepsize;
            
            offset = offset/tempOffSS;
            
            for(int a = 0; a < OffStepsize; a++) {
                Pstd += pow(P_ar[a] - offset, 2);
            }
            Pstd = Pstd / ( tempOffSS - 1 );
            
            roffset = roffset/tempOffSS;
            
            for(int a = 0; a < OffStepsize; a++) {
                rstd += pow(r_ar[a] - roffset, 2);
            }
            
            rstd = rstd / ( tempOffSS - 1 );
            
            for(int a = oldversions - 1; a > 0; a--) {  // save old versions
                offset_ar[a] = offset_ar[a-1];
                std_ar[a] = std_ar[a-1];
                //roffset_ar[a] = roffset_ar[a-1];
                //rstd_ar[a] = rstd_ar[a-1];
            }
            
            offset_ar[0] = offset;
            offset = 0;                 // reset the offset
            
            std_ar[0] = Pstd;
            Pstd = 0;
            
//            roffset_ar[0] = roffset;
//            roffset = 0
//
//            rstd_ar[0] = rstd;
//            rstd = 0;
            
        }
        
        
        if(inputRef > roffset + rstd * 100)
        {
            
            int ind = ResFind::indexofSmallestElement(std_ar, oldversions);
            
            offset = offset_ar[ind];
            
            if(std_ar[ind] > 190000) {         // if there hasn't been made a new offset
                offset = lasttimeoffset;        // use the old one
            }
            
            lasttimeoffset = offset;
            
            *mySetpoint = (uint16_t)offset;    // set the setpoint to the offset
            *HillPoin = roffset + rstd * 70;
            *AutoPoin = true;
            
            return true;
        }
        else {
            ResFind::Ramp();
            return false;
        }
        
        
    }
    else
        return false;
    
}
         


void ResFind::SetLimits(int outmin, int outmax)
{

    OutputMin = outmin;
    OutputMax = outmax;
}

//void ResFind::SetOffStepsize(int OffSS)
//{
    
//    OffStepsize = OffSS;
    //Pvector = std::vector<uint16_t>(OffStepsize);
    //rvector = std::vector<uint16_t>(OffStepsize);
    
//}
    
void ResFind::Ramp()
{
    int offset1 = 150;
    
    int maxnum = 4095-offset1-50;
    
    writecount++;
    
    if(writecount == RampSlow) {
        
        q++;
        
        if(q<maxnum) {
            outpu = q;
        }
        else {
            outpu = 2*maxnum - q;
        }
        
        if(q>maxnum*2-1) {
            q = 0;
        }
        
        writecount = 0;
        
    }
    
    *myOutput = outpu + offset1;
}

void ResFind::Initialize()
{
    offset = 0;
    Pstd = 0;
    roffset = 0;
    rstd = 0;
    for(int u = 0; u < oldversions; u++) {
        offset_ar[u] = 0;
        std_ar[u] = 200000;     // if you change this be sure to change
                                // the if(std_ar[ind] > 190000) condtion too
        //roffset_ar[u] = 0;
        //rstd_ar[u] = 0;
    }
    
    q += 100;
    
    // Sets initial values for the TakeMeThere function.
    
}

int ResFind::indexofSmallestElement(float array[], int size)
{
    int index = 0;
    
    for(int u = 1; u < size; u++)
    {
        if(array[u] < array[index])
            index = u;
    }
    
    return index;
}

