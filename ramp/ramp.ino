
#define PIN_OUTPUT A21

int offset1 = 150; // = 150; // 150 er min
int maxnum = 4095-offset1-50;

RampSlow = 1;

int writecount = 0, outpu = 0, q = 0;

uint16_t Output;


void setup() {
  pinMode(PIN_OUTPUT, OUTPUT);

}

void loop() {
  
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

    Output = outpu + offset1;
    
    analogWrite(PIN_OUTPUT, Output);

}
