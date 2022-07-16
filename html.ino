
void setupHtmlServer()
{
  //MAIN PAGE
  AsyncServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", MAIN_page);
      });
  //GESTION DES ENVOIS DE LA PAGE HTML
  AsyncServer.on("/wifiWipe",HTTP_GET, [](AsyncWebServerRequest *request){wm.resetSettings();ESP.restart();});
  AsyncServer.on("/set",HTTP_GET, [](AsyncWebServerRequest *request){handleSetParam(request);});
  AsyncServer.on("/motor-stop",HTTP_GET, [](AsyncWebServerRequest *request){
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Debug.println("motorStopped");
    request->send(200, "text/html", "<meta http-equiv=\"Refresh\" content=\"0; url=http://"+WiFi.localIP().toString()+"\">");
    });
  AsyncServer.on("/motor-up",HTTP_GET, [](AsyncWebServerRequest *request){motorUp(); request->send(200, "text/html", "<meta http-equiv=\"Refresh\" content=\"0; url=http://"+WiFi.localIP().toString()+"\">");});
  AsyncServer.on("/motor-down",HTTP_GET, [](AsyncWebServerRequest *request){motorDown(); request->send(200, "text/html", "<meta http-equiv=\"Refresh\" content=\"0; url=http://"+WiFi.localIP().toString()+"\">");});
  // Handle Web Server Events
  httpEvents.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
    String actualTime = String(hour())+":"+String(minute());
    String openingTime = String(openingTimeHour)+"h"+String(openingTimeMinute);
    String closingTime = String(closingTimeHour)+"h"+String(closingTimeMinute);
    httpEvents.send(actualTime.c_str(), "HEURE ACTUELLE");
    httpEvents.send(openingTime.c_str(), "HEURE OUVERTURE");
    httpEvents.send(closingTime.c_str(), "HEURE FERMETURE");
    httpEvents.send(String(openAfterSunrise).c_str(), "TIME-AFTER-SUNRISE");
    httpEvents.send(String(closeAfterSunset).c_str(), "TIME-AFTER-SUNSET");
    httpEvents.send(String(timeOfTimeout).c_str(), "TIME-UP");
    httpEvents.send(String(timeOfTimeout - timeOfTimeoutDownMinus).c_str(), "TIME-DOWN");
  });
  AsyncServer.addHandler(&httpEvents);


  //webSerial Setup
  WebSerial.begin(&AsyncServer);
  /* Attach Message Callback */
  WebSerial.msgCallback(recvMsg);

   //start server
  AsyncServer.begin();                  //Start server
  Debug.println("HTTP server started");
}

void handleSetParam(AsyncWebServerRequest* request){
  String inputMessage;
  
  if (request->hasParam("TIME-AFTER-SUNRISE")) {
    inputMessage = request->getParam("TIME-AFTER-SUNRISE")->value();
    openAfterSunrise = inputMessage.toInt();
    httpEvents.send(String(openAfterSunrise).c_str(), "TIME-AFTER-SUNRISE");
    calculerLeverCoucherSoleil();
    writeFile("/openAfterSunrise.txt", String(openAfterSunrise).c_str());
  }
  else if (request->hasParam("TIME-AFTER-SUNSET")) {
    inputMessage = request->getParam("TIME-AFTER-SUNSET")->value();
    closeAfterSunset = inputMessage.toInt();
    httpEvents.send(String(closeAfterSunset).c_str(), "TIME-AFTER-SUNSET");
    calculerLeverCoucherSoleil();
    writeFile("/closeAfterSunset.txt", String(closeAfterSunset).c_str());
  }

  else if (request->hasParam("TIME-UP")) {
    inputMessage = request->getParam("TIME-UP")->value();
    timeOfTimeout = inputMessage.toInt();
    httpEvents.send(String(timeOfTimeout).c_str(), "TIME-UP");
    Debug.println("temps de montée = "+String(timeOfTimeout));
    writeFile("/timeUp.txt", String(timeOfTimeout).c_str());
  }

  else if (request->hasParam("TIME-DOWN")) {
    inputMessage = request->getParam("TIME-DOWN")->value();
    timeOfTimeoutDownMinus = timeOfTimeout - inputMessage.toInt();
    httpEvents.send(inputMessage.c_str(), "TIME-DOWN");
    Debug.println("timeOfTimeoutDownMinus = "+String(timeOfTimeoutDownMinus));
    writeFile("/timeDown.txt", String(timeOfTimeoutDownMinus).c_str());
  }

  request->send(200, "text/html", "<meta http-equiv=\"Refresh\" content=\"0; url=http://"+WiFi.localIP().toString()+"\">");
}

/*
//Pour envoyer les infos avant la page web ne s'affiche (pas de "blanc" à la place des valeurs)
String processor(const String& var){
  if(var == "HEURE OUVERTURE"){
    return String(openingTimeHour)+"h"+String(openingTimeMinute);
  }
  else if(var == "HEURE FERMETURE"){
    return String(closingTimeHour)+"h"+String(closingTimeMinute);
  }
  else if(var == "HEURE ACTUELLE"){
    return String(hour())+":"+String(minute());
  }
  else if(var == "WIFI SETUP"){
    return "http://"+WiFi.localIP().toString()+":90";
  }
}
*/

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len){
  String incomingString = "";
  for(int i=0; i < len; i++){
    incomingString += char(data[i]);
  }
  
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
    Debug.println("motorStopped");
  }
  else if (incomingString.substring(0, 10) == "changeTime")
  {
    Debug.println("changeTime");
    int newTime = incomingString.substring((incomingString.indexOf('/'))+1, incomingString.length()).toInt();
    Debug.println("newTime = "+String(newTime));
    timeOfDuty = newTime;
  }
  else if(incomingString.substring(0, 11) == "testMotorUp")
  {
    Debug.println("MotorUp in 2sec");
    delay(2000);
    motorUp();
    delay(3000);
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Debug.println("motorStopped");
  }
  else if(incomingString.substring(0, 13) == "testMotorDown")
  {
    Debug.println("MotorDown in 2sec");
    delay(2000);
    motorDown();
    delay(3000);
    moving = false;
    digitalWrite(motor_plus, 0);
    digitalWrite(motor_moins, 0);
    digitalWrite(sendTension, 0);
    Debug.println("motorStopped");
  }
  else if(incomingString.substring(0, 10) == "testSensor")
  {
    digitalWrite(sendTension, 1);
    Debug.println("testing sensors...");
    unsigned long nowTime = millis();
    while(millis()-nowTime < 15000)
    {
      checkSensorUp();
      checkSensorDown();
      delay(100);
    }
    Debug.println("end of test sensor");
  }
}
