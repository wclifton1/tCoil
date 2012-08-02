//Program and Pair new BT module
//SeedStudio BT Bee
//GIVE UNIQUE ID TO EACH BT BEE

#define DEBUG_ENABLED  1

void setup() 
{
  setupBlueToothConnection();
} 

void loop() 
{ 
  //Typical Bluetoth command - response simulation:

  //Type 'a' from PC Bluetooth Serial Terminal
  //See Bluetooth Bee - Wiki for instructions

  if(Serial.read() == 'a')
  {
    Serial.println("You are connected");
    //You can write you BT communication logic here
  }

} 


void setupBlueToothConnection()
{
  Serial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  sendBlueToothCommand("\r\n+STWMOD=0\r\n");
  sendBlueToothCommand("\r\n+STNA=tCoil2\r\n"); //Make this unique for each new module
  sendBlueToothCommand("\r\n+STAUTO=1\r\n");
  sendBlueToothCommand("\r\n+STOAUT=1\r\n");
  sendBlueToothCommand("\r\n+STPIN=0000\r\n");
  sendBlueToothCommand("\r\n+RTADDR\r\n");
  delay(2000); // This delay is required.
  sendBlueToothCommand("\r\n+INQ=1\r\n");
  delay(3000);
}


void sendBlueToothCommand(char command[])
{
  Serial.print(command);
  delay(3000);
  //CheckOK();   

 // while(Serial.available())              //For debugging, Comment this line if not required  
 // {                                               //For debugging, Comment this line if not required   
 //   Serial.print(char(Serial.read()));  //For debugging, Comment this line if not required  
 // }   
}


