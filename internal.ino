// Internal Microcontroller Code
//TODO: needed?
char PWMSignal = 0;
char otherSignal = 0;
int sensorValue = 0;

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

int maxSensorVal = 640; //empirically calibrate, 100% battery
int minSensorVal = 465; //empirically calibrate, battery cutoff
int vScale = 154; // empirically determine (analog input/x = voltage) for two 100kOhm
int vScalePwr = 16; //empir. for 1MOhm then 100kOhm
int sysPower = 0; //empir power of system with motor off in mW. so we display only motor power draw

char SOP ='<';
char EOP ='>';

boolean started = false;
boolean ended = false; //DO NOT initialize each time the loop goes around, the loop happens faster than 9600baud sometimes
char inData[80];
byte index=0;

int i =0;

String stringFinal = "";

String msg = "";
String RPM = "";
String bat = "";
String batvolt = "";
String power = "";

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
  //Serial.println("startloop"); //DEBUG
  //read all serial data available, as fast as possible

  while(Serial.available() > 0){
    //Serial.println("serial available"); //DEBUG
    char inChar = Serial.read();
    if(inChar == SOP) {
      //Serial.println("SOP"); //DEBUG
      index=0;
      inData[index] = '\0';
      started = true;
      ended = false;
    }
    else if(inChar == EOP) {
      //Serial.println("EOP"); //DEBUG
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
        //Serial.println(">79"); //DEBUG
      }
    }
    //Serial.println(started); //DEBUG
    //Serial.println(ended); //DEBUG
    //we are here b/c end of packet arrived OR all data read
    //Serial.println("end data or all read"); //DEBUG
    if(started && ended) {
      //complete packet, process
      //Serial.println("received"); //DEBUG
      analogWrite(PWM_PIN, inData[0]); // make pwm signal (0-255)
      //reset for next packet
      //Serial.println("reset"); //DEBUG
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

    msg = coilCoupling();
    RPM = getFrequency(); //TODO
    bat = batteryFunction();
    batvolt = BatteryVoltage();
    power = Power();

    stringFinal = "";
    stringFinal += SOP;
    stringFinal += "in"; //for parsing source
    stringFinal += ",";
    stringFinal += bat;
    stringFinal += ",";
    stringFinal += batvolt;
    stringFinal += ",";
    stringFinal += RPM;
    stringFinal += ",";
    stringFinal += msg;
    stringFinal += ",";
    stringFinal += power;
    stringFinal += EOP;
    stringFinal +='.'; //extra
    char s1[stringFinal.length()];
    stringFinal.toCharArray(s1, stringFinal.length());
    Serial.println(s1);
  } 

  if (analogRead(PAIR_PIN)>700) {
    Serial.println("");
    Serial.println("+INQ=1"); //pairing code
    delay(30000);
    Serial.println("");
    Serial.println("+INQ=0");
  }
}

//power measurement function across defined resistor
String Power() { 
  float pwr3 = 0;
  float pwr4 = 0;
  float pwr5 = 0;
  float i3=0;
  float i4=0;
  float i5=0;
  
  for (i=0; i<40; i++){
    i3 += analogRead(PWR_BATT)/PWR_R;
    i4 += analogRead(PWR_COIL)/PWR_R;
    i5 += analogRead(PWR_MC)/PWR_R;
  }

  i3 = i3/40;
  i4 = i4/40;
  i5 = i5/40;
  
  pwr3 = i3 * 
  pwr = float(v*v)/vScalePwr/PWR_R*1000-sysPower; //equals power in mW

  if (pwr<0){
    pwr=0;
  }

  //Serial.println(pwr);

  char p[32];
  dtostrf(pwr,3,0,p);
  return (p);
  //Serial.println("endpower");
}

//analog measurement function
String coilCoupling() { //Voltage is > 1.5 means coupled
  float voltage = 0;
  int sensorValue = analogRead(COIL_PIN);
  voltage = (float)sensorValue/vScale; //empirically determined
  char v[32];
  dtostrf(voltage,5,2,v);
  return (v);
  //return("99");
}

// Internal Battery Voltage
String BatteryVoltage() {
  sensorValue = analogRead(BATT_PIN);
  float batvolt = 0;
  batvolt = (float)sensorValue/vScale;
  char s[32];
  dtostrf(batvolt, 5, 2, s);
  return(s);
  //return("99");
}

// Internal Battery Life
String batteryFunction() {
  int BatteryLife;
  BatteryLife= (100-100*(maxSensorVal-(float)sensorValue)/(maxSensorVal-minSensorVal)); //3.3V (480) - 4.5V (640) is 0-100%
  if (BatteryLife < 0) {
    BatteryLife = 0;
  }
  if (BatteryLife > 100){
    BatteryLife = 100;
  }
  char q[32];
  dtostrf(BatteryLife, 5, 2, q);
  return(q);
  //return("99");
}

// Get frequency function and take average
String getFrequency() {
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
  //return("99");
}

