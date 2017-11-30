
#define PIN_OUTBIG A21
#define PIN_OUTSMALL A22

int offset1 = 150; // 150 is min
int maxnum = 4095-offset1-50;

int RampSlowb = 100;
int RampSlows = 1; 

int writecountb = 0, outputb = 0, qb = 0;
int writecounts = 0, outputs = 0, qs = 0;

uint16_t OutBig, OutSmall;


void setup() {
  pinMode(PIN_OUTBIG, OUTPUT);
  pinMode(PIN_OUTSMALL, OUTPUT);
  analogWriteResolution(12);

}

void loop() {
  
    writecountb++;
    
    if(writecountb == RampSlowb) {
        
        qb++;
        
        if(qb<maxnum) {
            outputb = qb;
        }
        else {
            outputb = 2*maxnum - qb;
        }
        
        if(qb>maxnum*2-1) {
            qb = 0;
        }
        
        writecountb = 0;
        
    }

    writecounts++;
    
    if(writecounts == RampSlows) {
        
        qs++;
        
        if(qs<maxnum) {
            outputs = qs;
        }
        else {
            outputs = 2*maxnum - qs;
        }
        
        if(qs>maxnum*2-1) {
            qs = 0;
        }
        
        writecounts = 0;
        
    }

    OutBig = outputb + offset1;

    OutSmall = outputs + offset1;
    
    analogWrite(PIN_OUTBIG, OutBig);

    //analogWrite(PIN_OUTSMALL, OutSmall);

}
