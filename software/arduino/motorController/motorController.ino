#define BRAKEVCC 0
#define CW   1
#define CCW  2
#define BRAKEGND 3
#define CS_THRESHOLD 100

/*  VNH2SP30 pin definitions
 xxx[0] controls '1' outputs
 xxx[1] controls '2' outputs */
int inApin[2] = {7, 4};  // INA: Clockwise input
int inBpin[2] = {8, 9}; // INB: Counter-clockwise input
int pwmpin[2] = {5, 6}; // PWM input
int cspin[2] = {2, 3}; // CS: Current sense ANALOG input
int enpin[2] = {0, 1}; // EN: Status of switches output (Analog pin)
int statpin = 13;


void setup()
{
  pinMode(statpin, OUTPUT);
  // Initialize digital pins as outputs
  for (int i=0; i<2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwmpin[i], OUTPUT);
  }
  // Initialize braked
  for (int i=0; i<2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  digitalWrite(statpin, HIGH);
  Serial.begin(57600);      //the connection to the computer via USB
  Serial1.begin(57600);     //serial is the bluetooth from the controller
}    


long t0 = 0;
bool motoroff = true;
bool modeGamepad = false;

void loop() {
  long t1 = millis();
  int n = Serial1.available();
  if (n >= 4) {
    int16_t v[2] = {0}; //-128..127 
    Serial1.readBytes((char*)v, 4);

    if (v[0] == 1024){
      modeGamepad =! modeGamepad;
      return;    
    }
    if (modeGamepad){
      setThrottle(v[0], v[1]);
      t1 = millis();
      t0 = t1;
      motoroff=false;
    }         

  }
  n = Serial.available();
  if (n >= 4) {
    int16_t v[2] = {0}; //-128..127 
    Serial.readBytes((char*)v, 4);
    if (!modeGamepad) {
      setThrottle(v[0], v[1]);
      t1 = millis();
      t0 = t1;   
      motoroff=false;    
    }  
  }

  if (t1 - t0 > 100 && !motoroff) {
    int16_t v[2] = {0}; //-128..127 
    n = Serial1.available();
    if (n > 0) Serial1.readBytes((char*)v, n);
    n = Serial.available();
    if (n > 0) Serial.readBytes((char*)v, n);
    setThrottle(0, 0);  
    motoroff = true;    
  }  
  delay(10);
}


void motorOff(int motor)
{
  // Initialize braked
  for (int i=0; i<2; i++)
  {
    digitalWrite(inApin[i], LOW);
    digitalWrite(inBpin[i], LOW);
  }
  analogWrite(pwmpin[motor], 0);
}

/* motorGo() will set a motor going in a specific direction
 the motor will continue going in that direction, at that speed
 until told to do otherwise.
 
 motor: this should be either 0 or 1, will selet which of the two
 motors to be controlled
 
 direct: Should be between 0 and 3, with the following result
 0: Brake to VCC
 1: Clockwise
 2: CounterClockwise
 3: Brake to GND
 
 pwm: should be a value between ? and 1023, higher the number, the faster
 it'll go
 */
 void setThrottle(float TL, float TR) {
  if (TL >= 0) {
    digitalWrite(inApin[0], HIGH);
    digitalWrite(inBpin[0], LOW);
  } else {
    digitalWrite(inApin[0], LOW);
    digitalWrite(inBpin[0], HIGH);
  }

  if (TR >= 0) {
    digitalWrite(inApin[1], HIGH);
    digitalWrite(inBpin[1], LOW);
  }
  else {
    digitalWrite(inApin[1], LOW);
    digitalWrite(inBpin[1], HIGH);
  }
  analogWrite(pwmpin[0], abs(TL));
  analogWrite(pwmpin[1], abs(TR));
}


void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)
{
  if (motor <= 1)
  {
    if (direct <=4)
    {
      // Set inA[motor]
      if (direct <=1)
        digitalWrite(inApin[motor], HIGH);
      else
        digitalWrite(inApin[motor], LOW);

      // Set inB[motor]
      if ((direct==0)||(direct==2))
        digitalWrite(inBpin[motor], HIGH);
      else
        digitalWrite(inBpin[motor], LOW);

      analogWrite(pwmpin[motor], pwm);
    }
  }
}





//  if (n >= 4) {
//    int16_t v[2] = {0}; //-128..127 
//    Serial.readBytes((char*)v, 4);
//    setThrottle(v[0], v[1]);
//    //t1 = millis();
//    t0 = t1;
//    
//    Serial1.print(v[0]);
//    Serial1.print(" ");
//    Serial1.print(v[1]);
//    Serial1.print(" ");    
//    Serial1.print(n);
//    Serial1.println(" <<<<<<<<<<<<<<<<<<<<");
//    
//    motoroff=false;
//  }
//
//  //if (ix % 100 == 0)  Serial.println(ix);
//  if (t1 - t0 > 100 && !motoroff) {
//    int16_t v[2] = {0}; //-128..127 
//    n = Serial.available();
//    if (n > 0) Serial.readBytes((char*)v, n);
//     n = Serial.available();
//    setThrottle(0, 0);
//    Serial1.print(n);    
//    Serial1.println(" >>>>>>>>>>>>>>>>>>>");    
//  
//    motoroff = true;    
//  }  
//  delay(5);

