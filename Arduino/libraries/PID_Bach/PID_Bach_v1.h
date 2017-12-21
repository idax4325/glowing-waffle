#ifndef PID_Bach_v1_h
#define PID_Bach_v1_h
#define LIBRARY_VERSION	1.1.1

class PID
{


  public:

  //Constants used in some of the functions below
  #define AUTOMATIC	1
  #define MANUAL	0
  #define DIRECT  0
  #define REVERSE  1
  #define P_ON_M 0
  #define P_ON_E 1

  //commonly used functions **************************************************************************
    PID(uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, int*, bool*,  // * constructor.  links the PID to the Input, Output, and
        bool*, float*);//   Setpoint.  Initial tuning parameters are also set here.

    void SetMode();               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute();                       // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(int, int); // * clamps the output to a specific range. 0-255 by default, but
										                      //   it's likely the user will want to change this depending on
										                      //   the application
	

    
    
  // Public variables ****************************************************************
    
    float kp;                  // * (P)roportional Tuning Parameter
    float ki;                  // * (I)ntegral Tuning Parameter
    float kd;                  // * (D)erivative Tuning Parameter
    
    float ampcon;
    
    // Private variables ****************************************************************
    
  private:
	void Initialize();

	int pOn;

    float BSfac;
    
    uint16_t *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    uint16_t *myOutputB;             //   This creates a hard link between the variables and the
    uint16_t *mySetpoint;           //   PID, freeing the user from having to constantly tell us
    uint16_t *myOutputS;                              //   what these values are.  with pointers we'll just know.
    uint16_t *myInputRef;
    uint16_t *myVerboseNum;
    int *DirecPoin;
    bool *AutoPoin;
    float *HillPoin;
    
    float outputSum;
    uint16_t lastInput;

	uint16_t outMin, outMax;
	bool pOnE;
    bool lastAuto;
    bool* VP;
    
    bool small;
    
    int offcounter;
};
#endif

