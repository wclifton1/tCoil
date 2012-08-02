// External Battery (is this really a battery? was this designed for external battery use?)

void setup() {
  Serial.begin(9600);
}

void loop() {
  String StringBat = BatteryFunction2(A6);
  Serial.println(String(StringBat));
  delay(1000);
} 

String BatteryFunction2(int BatteryPin) {
  int sensorValue = analogRead(BatteryPin);//may need to add float
  float BatteryVoltage = float(sensorValue)/212;
  float BattVolt = BatteryVoltage*6; //why extra line?
  float BatteryLife = float(28.57*BattVolt - 528.5); //float() necessary? how did this formula come about?

  if (BatteryLife < 0)
    BatteryLife = 0;
  if (BatteryLife > 100)
    BatteryLife = 100;
  char b1[32];
  dtostrf(BatteryLife, 5, 2, b1); //look up this conversion
  char b2[32];
  dtostrf(BattVolt, 5, 2, b2);
  //Serial.print("Sensor Value =");
  //Serial.println(sensorValue);

  String s = String(b1) + ", ";
  s= s + String(b2); //currently outputting blank?
  return(s);
}

