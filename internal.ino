// Internal Arduino Code
// v11 7/11/12

const int PWM_PIN = 6; // digital pin output
const int LED = 13;
const int FREQ_PIN = A2;

long a0 = 0;
long a1 = 0;
long a3 = 0;
long a4 = 0;
long a5 = 0;
String RPM = "00000";
String stringFinal = "";

char SOP ='<';
char EOP ='>';

boolean started = false;
boolean ended = false; //DO NOT initialize each time the loop goes around, the loop happens faster than 9600baud sometimes
char inData[30];
byte index=0;

int i =0;

// Setup program
void setup() {
  Serial.begin(9600); //increase for speed?
  Serial.flush();
  pinMode(PWM_PIN, OUTPUT);
  pinMode(FREQ_PIN, INPUT); 
  pinMode(LED, OUTPUT);
  analogWrite(PWM_PIN, 255);  //turn on motor
}

// Main loop
void loop() {
  //Serial.println("startloop"); //DEBUG
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
      if (index < 29) {
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
      //Serial.println("received"); //DEBUG
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

    readData();
    RPM = getFrequency();

    stringFinal = "";
    stringFinal += SOP;
    stringFinal += String(a0);
    stringFinal += ',';
    stringFinal += String(a1);
    stringFinal += ',';
    stringFinal += String(RPM);
    stringFinal += ',';
    stringFinal += String(a3);
    stringFinal += ',';
    stringFinal += String(a4);
    stringFinal += ',';
    stringFinal += String(a5);
    stringFinal += EOP;
    stringFinal +='.'; //extra
    char s1[stringFinal.length()];
    stringFinal.toCharArray(s1, stringFinal.length());
    Serial.println(s1);
  } 

  if (analogRead(A7)>1000) {
    Serial.println("");
    Serial.println("+INQ=1"); //pairing code
    delay(30000);
    Serial.println("");
    Serial.println("+INQ=0"); //end pairing code
  }
}

//coil voltage
void readData() {
  //Serial.println("coil");

  for (i=0; i<40; i++){
    a0 += analogRead(A0);
    a1 += analogRead(A1);
    a3 += analogRead(A3);
    a4 += analogRead(A4);
    a5 += analogRead(A5);
  }
  a0 = a0/40*3300/1024;
  a1 = a1/40*3300/1024;
  a3 = a3/40*3300/1024;
  a4 = a4/40*3300/1024;
  a5 = a5/40*3300/1024;
}


// Get frequency function and take average
String getFrequency() {
  //Serial.println("fr");
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
  rpm = 60*1000000/pulse; //pulse units are us/cycle, cycles/us=1/pulse, 1000000/pulse=cycles/s, 60*1000000/pulse=rpm
  if (rpm < 0) {
    rpmout = "0";
  }
  else {
    rpmout = String(rpm);
  }
  return(rpmout);
}



