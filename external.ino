// External Battery 

void setup() {
  Serial.begin(9600);
}

void loop() {
  String StringBat = BatteryFunction2(A6);
  Serial.println(String(StringBat));
  delay(3000);
} 

String BatteryFunction2(int BatteryPin) {
  int sensorValue = analogRead(BatteryPin);//may need to add float
  float BatteryVoltage = (float)sensorValue/212;
  float BattVolt = BatteryVoltage*6;
  float BatteryLife = float(28.57*BattVolt - 528.5);
  
  if (BatteryLife < 0)
	BatteryLife = 0;
  if (BatteryLife > 100)
	BatteryLife = 100;
  char b1[32];
  dtostrf(BatteryLife, 5, 2, b1);
  char b2[32];
  dtostrf(BattVolt, 5, 2, b2);
  //Serial.print("Sensor Value =");
  //Serial.println(sensorValue);

  String s = "#ex,"+String(b1) + ",";
  s= s + String(b2)+",<!MSG>";
  return(s);
}

