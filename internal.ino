int PWM1 = 3;
int PWM2 = 5;
int PWM3 = 6;
int PWM4 = 9;
int PWM5 = 10;
int PWM6 = 11;
int led = 13;
void setup() {
  pinMode(PWM1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(PWM3, OUTPUT);
  pinMode(PWM4, OUTPUT);
  pinMode(PWM5, OUTPUT);
  pinMode(PWM6, OUTPUT);
  pinMode(led, OUTPUT);
  analogWrite(PWM1, 127);
  analogWrite(PWM2, 127); 
  analogWrite(PWM3, 127); 
  analogWrite(PWM4, 127); 
  analogWrite(PWM5, 127); 
  analogWrite(PWM6, 127);
  digitalWrite(led, LOW); 
}
void loop() {
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(1000);
}

