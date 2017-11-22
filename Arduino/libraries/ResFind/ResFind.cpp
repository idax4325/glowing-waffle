#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <ResFind.h>
#include <cmath>


ResFind::ResFind(uint16_t* Input, uint16_t* InputRef, uint16_t* Output, uint16_t* Setpoint, uint16_t* VerboseNum, bool* AutoAddr, bool* VerbAddr, float* HillAddr)
{
    myInput = Input;        // Pass pointer to the input and output to the ResFind
    myInputRef = InputRef;
    myOutput = Output;
    mySetpoint = Setpoint;
    myVerboseNum = VerboseNum;
    AutoPoin = AutoAddr;
    HillPoin = HillAddr;
    VP = VerbAddr;
    
    lasttimeoffset = 29000;
    lasttimestd = 30000;    //should just be a high value so it doesn't "win"
    
//    fin_roffset = 65000;
    
    OutputMax = 2500; // Set default output max and min
    OutputMin = 0;
    
    Direction = FORWARDS;   // Set default direction
    
    running = false;        // Set this boolean to true so it can be changed to false with the pause
                            // functions and thus stop the search if necessary
    allowSPchange = true;
    
    //iWait = 0;              // parameter that makes it possible to wait a bit with declaring resonance
                            // after seeing PDH start. Might not be useful
    
    RampSlow = 100;
    outpu = 0, q = 0;
    
    ResFind::Initialize();
    
}



void ResFind::Running()
{
    running = !running;
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
        
        offset += input/(float)OffStepsize;        // add a bit of the input to the offset
        P_ar[i] = input;
        
        i++;

//        roffset += inputRef;
//        r_ar[i] = inputRef;
        
//        if(inputRef > *HillPoin * 0.6) {
//            offset = 0;
////            roffset = 0;
//            i = 0;
//        }
        
        if(i % OffStepsize == 0) //&& i!= 0)            // is the offset done collecting inputs?
        {
            
            for(int a = 0; a < OffStepsize; a++) {
                Pstd += pow(P_ar[a] - offset, 2);
            }
            Pstd = Pstd / ( (float)OffStepsize - 1 );
            
//            roffset = roffset/tempOffSS;
//
//            for(int a = 0; a < OffStepsize; a++) {
//                rstd += pow(r_ar[a] - roffset, 2);
//            }
//
//            rstd = rstd / ( tempOffSS - 1 );
            
            for(int a = oldversions - 1; a > 0; a--) {  // save old versions
                offset_ar[a] = offset_ar[a-1];
                std_ar[a] = std_ar[a-1];
                //roffset_ar[a] = roffset_ar[a-1];
                //rstd_ar[a] = rstd_ar[a-1];
            }
            
            offset_ar[0] = offset;
            std_ar[0] = Pstd;
//            roffset_ar[0] = roffset;
//            rstd_ar[0] = rstd;
            
//            fin_roffset = roffset;
            
            offset = 0;
            Pstd = 0;
            i = 0;
//            roffset = 0
//            rstd = 0;
            
        }
        
        
        if(inputRef > *HillPoin * 0.8)
        {
            
            int ind = ResFind::indexofSmallestElement(std_ar, oldversions);
            
            if(std_ar[ind] > 19000) {         // if there hasn't been made a new offset
                offset = lasttimeoffset;      // use the old one
            }
            else if(std_ar[ind] > lasttimestd * 2){
                offset = lasttimeoffset;
            }
            else {
                offset = offset_ar[ind];
                lasttimestd = std_ar[ind];
                lasttimeoffset = offset;
            }
            if(allowSPchange) *mySetpoint = (uint16_t)offset;    // set the setpoint to the offset
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
    i = 0;
//    roffset = 0;
//    rstd = 0;
    
    
    for(int u = 0; u < oldversions; u++) {
        offset_ar[u] = 0;
        std_ar[u] = 20000;     // if you change this be sure to change
                                // the if(std_ar[ind] > 19000) condtion too
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

