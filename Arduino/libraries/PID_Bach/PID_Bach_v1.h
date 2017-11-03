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
    PID(uint16_t*, uint16_t*, uint16_t*, uint16_t*, uint16_t*, int*, bool*,  // * constructor.  links the PID to the Input, Output, and
        bool*, float*);//   Setpoint.  Initial tuning parameters are also set here.

    void SetMode();               // * sets PID to either Manual (0) or Auto (non-0)

    bool Compute();                       // * performs the PID calculation.  it should be
                                          //   called every time loop() cycles. ON/OFF and
                                          //   calculation frequency can be set using SetMode
                                          //   SetSampleTime respectively

    void SetOutputLimits(int, int); // * clamps the output to a specific range. 0-255 by default, but
										                      //   it's likely the user will want to change this depending on
										                      //   the application
	


  //available but not commonly used functions ********************************************************
	  

	void SetControllerDirection(int);	  // * Sets the Direction, or "Action" of the controller. DIRECT
										  //   means the output will increase when error is positive. REVERSE
										  //   means the opposite.  it's very unlikely that this will be needed
										  //   once it is set in the constructor.
    
    
										  
  //Display functions ****************************************************************
    
	int GetMode();						  //  inside the PID.
	int GetDirection();					  //
    
    
  // Public variables ****************************************************************
    
    int kp;                  // * (P)roportional Tuning Parameter
    int ki;                  // * (I)ntegral Tuning Parameter
    int kd;                  // * (D)erivative Tuning Parameter
    int controllerDirection;
    int sf;
    
    // Private variables ****************************************************************
    
  private:
	void Initialize();

	int pOn;

    uint16_t *myInput;              // * Pointers to the Input, Output, and Setpoint variables
    uint16_t *myOutput;             //   This creates a hard link between the variables and the
    uint16_t *mySetpoint;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.
    uint16_t *myInputRef;
    uint16_t *myVerboseNum;
    int *DirecPoin;
    bool *AutoPoin;
    float *HillPoin;
    
    int lastResFindOutput;

    int outputSum;
    int lastInput;

	unsigned long SampleTime;
	double outMin, outMax;
	bool pOnE;
    bool lastAuto;
    bool* VP;
    
    int offcounter;
};
#endif

