#ifndef ResFind_h
#define ResFind_h

class ResFind
{


  public:
    
    #define FORWARDS  1
    #define BACKWARDS  0
    
    
  // functions **************************************************************************
    ResFind(uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, bool*, bool*, float*);                       	// Constructor
    bool TakeMeThere();						   			// Starts finding the resonance and gives 1 when resonance has been found or if outputlimit has been reached
	void Running();						    // Start or stop TakeMeThere
    
    void SetLimits(int, int);                   // Set limits for the output in volts, min first
    //void SetOffStepsize(int);
    void Ramp();
    
    int indexofSmallestElement(float [], int);
    
     // The following variables are public so that they can be changed by writing myResFind.var = value
    
    int Direction;
    int RampSlow;
    
  private:
    
    void Initialize();
    
    uint16_t *myInput;            // Pointers to the Input, Output, and Setpoint variables
    uint16_t *myOutput;           // Smarter because the input doesn't have to be given
    uint16_t *mySetpoint;         // everytime TakeMeThere is called
    uint16_t *myInputRef;
    uint16_t *myVerboseNum;
    bool *AutoPoin;
    float *HillPoin;
    int OutputMax;
    int OutputMin;
    bool running;
    uint32_t i;
    const static int oldversions = 4;
    float offset, Pstd;
    float offset_ar [oldversions] = {};
    float lasttimeoffset;
    float std_ar [oldversions] = {};
    //float roffset, rstd;
    //float roffset_ar [oldversions];
    //float rstd_ar [oldversions];
    //float fin_roffset;
    int PDHbegini;
    float input;
    float inputRef;
    float output;
    bool* VP;
    int writecount, outpu, q;
    const static int OffStepsize = 30;
//    uint16_t r_ar [OffStepsize];
    uint16_t P_ar [OffStepsize];

    
};
#endif

