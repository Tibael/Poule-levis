void serialRead()
{
  String incomingString = "zero";

  incomingString = Serial.readString();

  
  if (incomingString.substring(0, 9) == "motorDown")
  {
    motorDown();
  }
  else if (incomingString.substring(0, 7) == "motorUp")
  {
    motorUp();
  }
  else if (incomingString.substring(0, 4) == "stop")
  {
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Serial.println("motorStopped");
    WebSerial.println("motorStopped");
  }
  else if (incomingString.substring(0, 10) == "changeTime")
  {
    Serial.println("changeTime");
    WebSerial.println("changeTime");
    int newTime = incomingString.substring((incomingString.indexOf('/'))+1, incomingString.length()).toInt();
    Serial.println("newTime = "+String(newTime));
    WebSerial.println("newTime = "+String(newTime));
    timeOfDuty = newTime;
  }
  else if(incomingString.substring(0, 11) == "testMotorUp")
  {
    Serial.println("MotorUp in 2sec");
    WebSerial.println("MotorUp in 2sec");
    delay(2000);
    motorUp();
    delay(3000);
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Serial.println("motorStopped");
    WebSerial.println("motorStopped");
  }
  else if(incomingString.substring(0, 13) == "testMotorDown")
  {
    Serial.println("MotorDown in 2sec");
    WebSerial.println("MotorDown in 2sec");
    delay(2000);
    motorDown();
    delay(3000);
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Serial.println("motorStopped");
    WebSerial.println("motorStopped");
  }
  else if(incomingString.substring(0, 10) == "testSensor")
  {
    digitalWrite(sendTension, 1);
    Serial.println("testing sensors...");
    WebSerial.println("testing sensors...");
    unsigned long nowTime = millis();
    while(millis()-nowTime < 15000)
    {
      checkSensorUp();
      checkSensorDown();
      delay(100);
    }
    Serial.println("end of test sensor");
    WebSerial.println("end of test sensor");
  }
}
