// Internal Microcontroller Code
char PWMSignal = 0;
char otherSignal = 0;
int sensorValue = 0;

const int PWM_PIN = 3; // digital pin output
const int FREQ_PIN = A5;
const int BAT_PIN = A3;
const int STATUS_PIN = A6;

char SOP ='<';
char EOP ='>';

boolean started = false;
boolean ended = false;
char inData[80];
byte index;

String stringFinal = "";

String msg = "";
String RPM = "";
String bat = "";
String batvolt = "";

// Setup program
void setup() {
  Serial.begin(9600); //increase for speed?
  Serial.flush();
  pinMode(PWM_PIN, OUTPUT);
  pinMode(FREQ_PIN, INPUT); 
  analogWrite(PWM_PIN, 0);  //turn off motor
}

// Main loop
void loop() {
  sensorValue = analogRead(BAT_PIN); 

  //read all serial data available, as fast as possible
  while(Serial.available() > 0){
    char inChar = Serial.read();
    //Serial.println(inChar);
    if(inChar == SOP) {
      index=0;
      inData[index] = '\0';
      started = true;
      ended = false;
    }
    else if(inChar == EOP) {
      ended = true;
      break;
    }
    else {
      if (index < 79) {
        inData[index] = inChar;
        index++;
        inData[index] = '\0';
      }
    }
  }
  //we are here b/c end of packet arrived OR all data read
  if(started && ended) {
    //complete packet, process
    Serial.println(inData);
    analogWrite(PWM_PIN, inData[0]); // make pwm signal (0-255)

    //reset for next packet
    started = false;
    ended = false;
    index = 0;
    inData[index] = '\0';
  }
  if (Serial.available() == 0) {
    msg = DeviceCoupling(STATUS_PIN);
    RPM = getFrequency(FREQ_PIN);
    bat = String(BatteryFunction());
    batvolt = String(BatteryVoltage());

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
    stringFinal += EOP;
    stringFinal +='.'; //extra

    char s1[stringFinal.length()];
    stringFinal.toCharArray(s1, stringFinal.length());
    Serial.println(s1);
  }
}

//analog measurement function
String DeviceCoupling(int AnalogInPin) { //Voltage is > 1.5 means coupled
  float Voltage = 0;
  String msg;
  int sensorValue = analogRead(AnalogInPin);
  Voltage = (float)sensorValue/214;
  Serial.println(Voltage);
  if(Voltage > 1.5)
    msg = "1";
  else
    msg = "0";
  return msg;
}

// Internal Battery Voltage
String BatteryVoltage() {
  float batvolt = 0;
  batvolt = (float)sensorValue/214;
  char s[32];
  dtostrf(batvolt, 5, 2, s);
  return(s);
}

// Internal Battery Life
String BatteryFunction() {
  float BatteryLife;
  float BatteryVoltage;

  BatteryVoltage = (float)sensorValue/214;
  BatteryLife= (111 * BatteryVoltage - 310); // 2.8V-3.7V is 0-100%
  if (BatteryLife < 0) 
    BatteryLife = 0;
  if (BatteryLife > 100)
    BatteryLife = 100;

  char s[32];
  dtostrf(BatteryLife, 5, 2, s);
  return(s);
}

// Get frequency function and take average
String getFrequency(int FREQ_PIN) {
  long pulse = 0;
  long rpm = 0; //could make this int by dividing by 1000
  long pulse1 = 0;
  long pulse2 = 0;
  int samples = 10;
  String rpmout = "";
  for(int j=0; j<samples; j++) pulse1+= pulseIn(FREQ_PIN, HIGH, 40000);
  for(int j=0; j<samples; j++) pulse2+= pulseIn(FREQ_PIN, LOW, 40000); //this timeout delays the whole loop, previously 250000
  pulse = pulse1 + pulse2;
  pulse = pulse/10;
  //float temprpm = float(33012-.7401*pulse);
  float temprpm = float(60*1000000/pulse); //pulse units are us/cycle, cycles/us=1/pulse, 1000000/pulse=cycles/s, 60*1000000/pulse=rpm
  rpm = long(temprpm); //int() not large enough
  //Serial.println(temprpm);
  //Serial.println(rpm);
  if (rpm < 0) 
    rpmout = "Motor is not running.";
  else {
    rpm = rpm/1000;
    rpmout = String(rpm)+"000";
  }
  return(rpmout);
}



