// Internal Microcontroller Code

const int PWM_PIN = 5; // digital pin output
const int LED = 13;
const int BATT_PIN = A0;
const int COIL_PIN = A1;
const int FREQ_PIN = A2;
const int PWR_BATT = A3;
const int PWR_COIL = A4;
const int PWR_MC = A5;
const int PAIR_PIN = A7; //give this pin VCC to enter bluetooth pair mode for 30 seconds

float PWR_R = 0.5;

float battVolt = 0;
float coilVolt = 0;
int battSensor = 0;
char p3[32];
char p4[32];
char p5[32];

int maxSensorVal = 640; //empirically calibrate, 100% battery
int minSensorVal = 465; //empirically calibrate, battery cutoff
int vScale = 154; // empirically determine (analog input/x = voltage) for two 100kOhm
int vScalePwr = 16; //empir. for 1MOhm then 100kOhm

char SOP ='<';
char EOP ='>';

boolean started = false;
boolean ended = false; //DO NOT initialize each time the loop goes around, the loop happens faster than 9600baud sometimes
char inData[80];
byte index=0;

int i =0;

String stringFinal = "";

String coilVolt_str = "";
String RPM = "";
String battFxn= "";
String battVolt_str = "";

// Setup program
void setup() {
  Serial.begin(9600); //increase for speed?
  Serial.flush();
  pinMode(PWM_PIN, OUTPUT);
  pinMode(FREQ_PIN, INPUT); 
  pinMode(LED, OUTPUT);
  analogWrite(PWM_PIN, 50);  //turn on motor
}

// Main loop
void loop() {
  Serial.println("startloop"); //DEBUG
  //read all serial data available, as fast as possible

  while(Serial.available() > 0){
    char inChar = Serial.read();
    if(inChar == SOP) {
      index=0;
      inData[index] = '\0';
      started = true;
      ended = false;
    }
    else if(inChar == EOP) {
      ended = true;
    }
    else {
      if (index < 79) {
        //Serial.println("inChar"); //DEBUG causes echo error to bluetooth
        inData[index] = inChar;
        index++;
        inData[index] = '\0';
      }
      else {
        index=0;
        started=false;
      }
    }

    //we are here b/c end of packet arrived OR all data read
    if(started && ended) {
      //complete packet, process
      Serial.println("received"); //DEBUG
      analogWrite(PWM_PIN, inData[0]); // make pwm signal (0-255)
      //reset for next packet
      started = false;
      ended = false;
      index = 0;
      inData[index] = '\0';
    }
  }

  if (Serial.available() == 0) { //this if is not really necessary, but may decrease lag
    digitalWrite(LED, HIGH);   // set the LED on
    delay (500); //in order to not create too many data points for the app to process, crashing android processing after 5 min with
    digitalWrite(LED, LOW);   // set the LED off

    coilVolt_str = coilCoupling();
    battVolt_str = BatteryVoltage();
    battFxn = batteryFunction();
    Serial.println(battFxn);
    power();
    RPM = getFrequency();
    Serial.println("makestring");

    stringFinal = "";
    Serial.println(stringFinal);
    stringFinal += SOP;
    Serial.println(stringFinal);
    stringFinal += "in;"; //for parsing source
    Serial.println(stringFinal);
    stringFinal += battFxn;
    Serial.println(stringFinal);
    stringFinal += 'z';
    stringFinal += battVolt_str;
    Serial.println(stringFinal);
    //stringFinal += ';';
    //stringFinal += RPM;
    //Serial.println(stringFinal);
    //stringFinal += ';';
    stringFinal += coilVolt_str;
    Serial.println(stringFinal);
    //stringFinal += ';';
    stringFinal += p3;
    Serial.println(stringFinal);
    //stringFinal += ';';
    stringFinal += p4;
    Serial.println(stringFinal);
    //stringFinal += ';';
    stringFinal += p5;
    Serial.println(stringFinal);
    stringFinal += EOP;
    Serial.println(stringFinal);
    stringFinal +='.'; //extra
    Serial.println(stringFinal);
    char s1[stringFinal.length()];
    stringFinal.toCharArray(s1, stringFinal.length());
    Serial.println(s1);
  } 

  if (analogRead(PAIR_PIN)>1000) {
    Serial.println("");
    Serial.println("+INQ=1"); //pairing code
    delay(30000);
    Serial.println("");
    Serial.println("+INQ=0"); //end pairing code
  }
}

//coil voltage
String coilCoupling() {
  Serial.println("coil");
  int coilSensor = 0;
  for (i=0; i<40; i++){
    coilSensor += analogRead(COIL_PIN);
  }
  coilSensor = coilSensor/40;

  coilVolt = (float)coilSensor/vScale; //empirically determined
  char v[32];
  dtostrf(coilVolt,5,2,v);
  return (v);
}

// Internal Battery Voltage
String BatteryVoltage() {
  Serial.println("voltage");
  for (i=0; i<40; i++){
    battSensor += analogRead(BATT_PIN);
  }
  battSensor = battSensor/40;

  battVolt = (float)battSensor/vScale;
  char s[32];
  dtostrf(battVolt, 5, 2, s);
  return(s);
}

// Internal Battery Life
String batteryFunction() {
  Serial.println("fxn");
  int BatteryLife;
  BatteryLife = (100-100*(maxSensorVal-(float)battSensor)/(maxSensorVal-minSensorVal)); //3.3V (480) - 4.5V (640) is 0-100%
//  if (BatteryLife < 0) {
//    BatteryLife = 0;
//  }
//  if (BatteryLife > 100){
//    BatteryLife = 100;
//  }
  char q[32];
  dtostrf(BatteryLife, 2, 0, q);
  return(q);
}

//power measurement function across defined resistor
void power() { 
  Serial.println("power");
  float pwrBatt = 0;
  float pwrCoil = 0;
  float pwrMC = 0;
  float iBatt=0;
  float iCoil=0;
  float iMC=0;

  for (i=0; i<40; i++){
    iBatt += analogRead(PWR_BATT);
    iCoil += analogRead(PWR_COIL);
    iMC += analogRead(PWR_MC);
  }

  iBatt = iBatt/40/PWR_R; //average and convert voltage to current
  iCoil = iCoil/40/PWR_R;
  iMC = iMC/40/PWR_R;

  Serial.println("iBatt");
  //Serial.println(iCoil);
  //Serial.println(iMC);

  pwrBatt = iBatt * battVolt * 1000; //equals power in mW
  pwrCoil = iCoil * coilVolt * 1000;
  pwrMC = iMC * 12 * 1000;

  if (pwrCoil<0) pwrCoil=0;
  if (pwrBatt<0) pwrBatt=0;
  if (pwrMC<0) pwrMC=0;

  dtostrf(pwrCoil,3,0,p3);
  dtostrf(pwrBatt,3,0,p4);
  dtostrf(pwrMC,3,0,p5);
}

// Get frequency function and take average
String getFrequency() {
  Serial.println("freq");
  long pulse = 0;
  long rpm = 0; //could make this int by dividing by 1000
  long pulse1 = 0;
  long pulse2 = 0;
  int samples = 10;
  String rpmout = "";
  for(int j=0; j<samples; j++) pulse1+= pulseIn(FREQ_PIN, HIGH, 40000); //returns length of 10 high pulses
  for(int j=0; j<samples; j++) pulse2+= pulseIn(FREQ_PIN, LOW, 40000); //returns length of 10 low pulses
  pulse = pulse1 + pulse2; //length of 10 rotations if sensor is set to 1/revolution, currently set that way
  pulse = pulse/10;
  float temprpm = float(60*1000000/pulse); //pulse units are us/cycle, cycles/us=1/pulse, 1000000/pulse=cycles/s, 60*1000000/pulse=rpm
  rpm = long(temprpm); //int() not large enough
  if (rpm < 0) {
    rpmout = "0";
  }
  else {
    rpm = rpm/1000;
    rpmout = String(rpm)+"000";
  }
  return(rpmout);
}


