void motorUp()
{
  isOpen = false;
  isClosed = true;
  Debug.println("motorUp");
  timeOut = millis();
  moving = true;
  digitalWrite(motor_moins, 0);
  digitalWrite(sendTension, 1);
  digitalWrite(motor_plus, 1);
}

void motorDown()
{
  isClosed = false;
  isOpen = true;
  Debug.println("motor Down");
  timeOut = millis();
  moving = true;
  digitalWrite(motor_plus, 0);
  digitalWrite(sendTension, 1);
  digitalWrite(motor_moins, 1);
}

void checkSensorUp()
{
  if(readAnalog(3) > 800)
  {
    digitalWrite(motor_plus, 0);
    digitalWrite(sendTension, 0);
    Debug.println("le poule-levis s'est fermé en "+String((millis()-timeOut)/1000)+"secondes");
    Debug.println("SensorUp activated");
    moving = false;
  }
}

void checkSensorDown()
{
  if(readAnalog(sensorDown) > 800)
  {
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Debug.println("le poule-levis s'est ouvert en "+String((millis()-timeOut)/1000)+"secondes");
    Debug.println("SensorDown activated");
    moving = false;
  }
}
