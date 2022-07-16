#include <ArduinoOTA.h>
#include <CertStoreBearSSL.h>
#include <ESP_OTA_GitHub.h>
#include <EMailSender.h>
#include <WiFiUdp.h>
#include <ESPAsyncWiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <WebSerial.h>
#include <FS.h>
#include <LittleFS.h>
#include <TimeLib.h>
#include <Ticker.h>
#include "index.h"

#define Debug WebSerial

#define GHOTA_CURRENT_TAG "2.1.0"

//----------------------------------------------------
//        VARIABLE D'AJUSTEMENT
//----------------------------------------------------
static int timeOfTimeout = 36;
static int timeOfTimeoutDownMinus = 22; //timeofTimeOutDown = timeOfTimeout - timeOfTimeoutDownMinus
static int timeOfTimeoutBackupAlim = 40;//5;
String emailAdressToNotify[] = {"david8.thom@gmail.com, athomasset11@gmail.com"};
static int nbAdressMail = 2;
IPAddress APIp (10,10,10,10); 
time_t timeOfSleep = 2*3600; //2h
double longitude = -4.584266;
double latitude = 46.006565;
bool deepSleepActivated = false;
bool pileActivated = false;
double lowBattery = 6.0;
bool do_update_check = true;
#define UPDATE_CHECK_INTERVAL_MS 60000

WiFiUDP Udp;
static const char ntpServerName[] = "us.pool.ntp.org";
int timeZone = 2;     // Central European Time
time_t getNtpTime();
unsigned int localPort = 8888;  // local port to listen for UDP packets
String emailAddress;
String emailPassword;
EMailSender emailSend ("", "");

//HTTP OTA
AsyncWebServer AsyncServer(80);
AsyncEventSource httpEvents("/events");

//WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
DNSServer dns;
AsyncWiFiManager wm(&AsyncServer, &dns);
bool asBootInAPmode = false;

//pins
int motor_moins = D4;
int motor_plus = D2;
int sendTension = D8;
int noWifiLed = D3;
int S0 = D7;
int S1 = D6;
int S2 = D5;
int batteryLevelPin = 0;
int backupAlim = 1;
int sensorDown = 2;
static int sensorUp = 3;
int sleepModeActivated = 4;

int timeOfDuty = 10;
bool moving = false;

int openingTimeHour = 06;
int openingTimeMinute = 00;
int closingTimeHour = 22;
int closingTimeMinute = 00;
int openAfterSunrise = 1;
int closeAfterSunset = 0;
time_t openTime;
time_t closeTime;
double leverSoleil;
double coucherSoleil;
double meridien;
int _day = 0;

bool isOpen = true;
bool isClosed = true;

//noWifi variables
Ticker ledBlink;
void ledBlinkFunc(){int state = digitalRead(noWifiLed);  digitalWrite(noWifiLed, !state);};

//time variables
int timeOut = 0; //timeOut du moteur
int timeSinceSync = 10001; //pour savoir si on redemande l'heure -- valeur : permet de l'executer à la premiere loop
int timeNoneVIPFonctions = 120001; //pour les fonctions qui s'executent toutes les 2 mins -- valeur : permet de l'executer à la premiere loop
time_t timeGoingToSleep;
bool initTime = true;
bool ntpStatus = false;
int ntpStatusLong = 0;
long lastCheckMillis = -UPDATE_CHECK_INTERVAL_MS;

bool isOnBackupAlim = false;
bool isLowBattery = false;

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);

  pinMode(motor_plus, OUTPUT);
  digitalWrite(motor_plus, 0);
  pinMode(motor_moins, OUTPUT);
  digitalWrite(motor_moins, 0);
  pinMode(sendTension, OUTPUT);
  digitalWrite(sendTension, 0);

  pinMode(noWifiLed, OUTPUT);
  digitalWrite(noWifiLed, 0);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  setupWifi();
  setupHtmlServer();

  //flashMem setup
  if (!LittleFS.begin()) {
    Debug.println("LittleFS mount failed");
    LittleFS.format();
    LittleFS.begin();
  }
  LittleFS.setTimeCallback(now);
  
  setupOTA();

  Serial.println("version : "+String(GHOTA_CURRENT_TAG));
  
  restoreVariable();

  // Démarrage du client NTP - Start NTP client
  Udp.begin(localPort);
  readTime();
  //Le chipset n'a pas pas accédé à l'heure sur internet
  if(!ntpStatus)
  {
    File file = LittleFS.open("/savedTime.txt", "r");
    //le chipset n'a jamais eu l'heure
    if (!file) 
    {
      Debug.println("Je n'ai jamais eu l'heure !");
    }
    //le chipset à eu l'heure à un moment
    else
    {
      setTime(file.getCreationTime()+(millis()/1000));
      file.close();
    }
  }
  initTime = false;
}

void loop() {
  //----------------------------------------------------
  //      GESTION DE L'ARRET MOTEUR
  //----------------------------------------------------
  if (moving)
  {
    serialRead();
    int localTimeout;
    if (isClosed)
    {
      //Debug.println("check Sensor Up");
      checkSensorUp();
      if(isOnBackupAlim)
      {
        localTimeout = timeOfTimeoutBackupAlim;
      }
      else
      {
        localTimeout = timeOfTimeout;
      }
    }
    else
    {
      //Debug.println("check Sensor Down");
      checkSensorDown();
      if(isOnBackupAlim)
      {
        localTimeout = timeOfTimeoutBackupAlim - timeOfTimeoutDownMinus;;
      }
      else
      {
        localTimeout = timeOfTimeout - timeOfTimeoutDownMinus;
      }
    }
    if (millis() - timeOut >= localTimeout*1000)
    {
      Debug.println("TimeOut");
      if(isOpen)
      {
        digitalWrite(motor_moins, 0);
        digitalWrite(sendTension, 0);
        moving = false;
      }
      else
      {
        digitalWrite(motor_plus, 0);
        digitalWrite(sendTension, 0);
        moving = false;
      }
      /*
      EMailSender::EMailMessage messageError;
      messageError.subject = "Erreur au Poule-Levis";
      messageError.message = "Salut ! <br>J'ai rencontré une erreur en me fermant ou m'ouvrant, peux-tu passer me voir ? :)";
  
      for (int i = 0; i < nbAdressMail; i++)
      {
        EMailSender::Response resp = emailSend.send(emailAdressToNotify[i], messageError);
        Debug.println("Sending status: ");
        Debug.println(resp.code);
        Debug.println(resp.desc);
        Debug.println(resp.status);
      }
      Debug.println("Envoi d'email : erreur d'ouverture ou de fermeture, boucle alim principale");
      */
    }
  }
  else
  {
    //----------------------------------------------------
    //       HTML AND SERIAL PROCESSING
    //----------------------------------------------------
    
    //DEBUG : si demarre en mode AP, reboot une fois le wifi rentré pour démarrer correctement les serveurs http
    if(WiFi.getMode() == WIFI_AP)
    {
       asBootInAPmode = true;
    }
    if(asBootInAPmode && WiFi.getMode()==WIFI_STA)
    {
      Debug.println("Restarting the board");
      ESP.restart();
    }
    
    serialRead();
    ArduinoOTA.handle();

    //----------------------------------------------------
    //        CHECK IF UPDATE ON GITHUB
    //----------------------------------------------------
    
    long currentMillis = millis();
    if (do_update_check && currentMillis - lastCheckMillis > UPDATE_CHECK_INTERVAL_MS) {
      handle_upgade();
      lastCheckMillis = currentMillis;
    }

    
    //==============================================
    //  FONCTIONS EXECUTÉS TOUTES LES 10 SECS
    //==============================================
    if (millis() - timeSinceSync > 10000)
    {
      //----------------------------------------------------
      //        MAJ DE L'HEURE
      //----------------------------------------------------
      readTime();
      
      //envois l'heure au serveur http
      String actualTime = String(hour())+":"+String(minute());
      httpEvents.send(actualTime.c_str(), "HEURE ACTUELLE");
      
      double batteryLevel = (readAnalog(batteryLevelPin)*(3.3/1024))/0.3125;
      Debug.print(batteryLevel);
      Debug.println("V");

      //----------------------------------------------------
      //      GESTION DU DEPART MOTEUR
      //----------------------------------------------------
      if (hour() == openingTimeHour and minute() == openingTimeMinute and isClosed)
      {
        motorDown();
      }
      else if (hour() == closingTimeHour and minute() == closingTimeMinute and isOpen)
      {
        motorUp();
      }

      //----------------------------------------------------
      //      GESTION DE LED NOWIFI
      //----------------------------------------------------
      if(ntpStatusLong > 10)
      {
        ledBlink.attach(0.5, ledBlinkFunc);
      }
      else if(ntpStatusLong == 0)
      {
        ledBlink.detach();
      }
    }
    
    //==============================================
    //  FONCTIONS EXECUTÉS TOUTES LES 2 MINS
    //==============================================
    if(millis() - timeNoneVIPFonctions > 120000)
    {
      timeNoneVIPFonctions = millis();
      //----------------------------------------------------------------------------
      //      CALCUL DE L'HEURE D'OUVERTURE ET DE FERMETURE : 1 fois par jour + CHANGEMENT HEURE ÉTÉ/HIVER
      //----------------------------------------------------------------------------
      if (_day != day())
      {
        _day = day();
        calculerLeverCoucherSoleil();
      }
      /*
      //----------------------------------------------------------------------------
      //                  CHANGEMENT HEURE ÉTÉ/HIVER
      //----------------------------------------------------------------------------
      //été : entre mars et octobre
      if(month() > 3 && month() < 10)
      {
        timeZone = 2;
      }
      //hiver : entre octobre et mars
      else if(month() < 3 || month() > 10)
      {
        timeZone = 1;
      }
      //si octobre : changement le dernier dimanche
      else if (month == 10)
      {
        if (day()<=24)
        {
          timeZone = 2;
        }
        else
        {
          TimeElements timeNow;
          breakTime(now(), timeNow);
          if(timeNow.Wday == 6) //si on est dimanche
          {
            timeZone = 1;
          }
        }  
      }
      //si mars : changement le dernier dimanche
      else if(month() == 3)
      {
        if(day()<=24)
        {
          timeZone = 1;
        }
      }
      */
      //----------------------------------------------------------------------------
      //      GESTION DE L'ALIMENTATION : BACKUP ALIM CHECK, BATTERY LEVEL CHECK
      //----------------------------------------------------------------------------
      if(readAnalog(backupAlim) < 300 && pileActivated) //si on est passé sur pile
      {
        //si ce n'était pas le cas il y a 2min
        if(!isOnBackupAlim)
        {
          isOnBackupAlim = true;
        
          //allumage de la led en continu
          ledBlink.detach();
          digitalWrite(noWifiLed, 1);
  
          
          //envois d'un mail
          EMailSender::EMailMessage messageError;
          messageError.subject = "ATTENTION : alimentation du Poule-Levis";
          messageError.message = "Salut ! <br>Je suis passé sur pile, tu devrais venir voir si mon alim n'a pas été débranché ! :/";
  
          Debug.println("Envoi d'email : passage sur pile");
          for (int i = 0; i < nbAdressMail; i++)
          {
            EMailSender::Response resp = emailSend.send(emailAdressToNotify[i], messageError);
            Debug.println("Sending status: ");
            Debug.println(resp.code);
            Debug.println(resp.desc);
            Debug.println(resp.status);
          }
        }
        else if (pileActivated)
        {
          double batteryLevel = (readAnalog(batteryLevelPin)*(3.3/1024))/0.3125;
          Debug.print(batteryLevel);
          Debug.println("V");
          if(batteryLevel <= lowBattery && !isLowBattery)
          {
            isLowBattery = true;
            //envois d'un mail
            EMailSender::EMailMessage messageError;
            messageError.subject = "ATTENTION : changer pile du Poule-Levis";
            messageError.message = "Salut ! <br>Il faudrait changer le pile 9V sur le poule-levis !";
    
            Debug.println("Envoi d'email : changer la pile");
            for (int i = 0; i < nbAdressMail; i++)
            {
              EMailSender::Response resp = emailSend.send(emailAdressToNotify[i], messageError);
              Debug.println("Sending status: ");
              Debug.println(resp.code);
              Debug.println(resp.desc);
              Debug.println(resp.status);
            }
          }
          else if (batteryLevel >= lowBattery+1.5)
          {
            isLowBattery = false;
          }
        }
      }
      else
      {
        isOnBackupAlim = false;
        digitalWrite(noWifiLed, 0);
        double batteryLevel = (readAnalog(batteryLevelPin)*(3.3/1024))/0.3125;
        Debug.print(batteryLevel);
        Debug.println("V");
        if(batteryLevel <= lowBattery && !isLowBattery && pileActivated)
        {
          isLowBattery = true;
          //envois d'un mail
          EMailSender::EMailMessage messageError;
          messageError.subject = "ATTENTION : changer pile du Poule-Levis";
          messageError.message = "Salut ! <br>Il faudrait changer le pile 9V sur le poule-levis !";
  
          Debug.println("Envoi d'email : changer la pile");
          for (int i = 0; i < nbAdressMail; i++)
          {
            EMailSender::Response resp = emailSend.send(emailAdressToNotify[i], messageError);
            Debug.println("Sending status: ");
            Debug.println(resp.code);
            Debug.println(resp.desc);
            Debug.println(resp.status);
          }
        }
        else if (batteryLevel >= lowBattery+1.5)
        {
          isLowBattery = false;
        }
      }

      //----------------------------------------------------
      //      GESTION DU DEEPSLEEP
      //----------------------------------------------------
      if(isOnBackupAlim && deepSleepActivated)
      {
        if(abs(openTime+300 - now()) <= timeOfSleep) //300 --> open/closeTime + 5min
        {
          digitalClockDisplay();
          int timeSleep = openTime - now() - 150; //reveil 2min30 avant ouverture ou fermeture
          timeGoingToSleep = timeSleep;
          LittleFS.setTimeCallback(sleepingTime);
          deleteFile("/savedTime.txt");
          writeFile("/savedTime.txt", "0");
          Debug.println("going to deepSleep open during "+String(timeSleep/60)+" minutes");
          ESP.deepSleep(timeSleep*1000000);
        }
        else if (abs(closeTime+300 - now()) <= timeOfSleep)
        {
          digitalClockDisplay();
          int timeSleep = closeTime - now() - 150; //reveil 2min30 avant ouverture ou fermeture
          timeGoingToSleep = timeSleep;
          LittleFS.setTimeCallback(sleepingTime);
          deleteFile("/savedTime.txt");
          writeFile("/savedTime.txt", "0");
          Debug.println("going to deepSleep close during "+String(timeSleep/60)+" minutes");
          ESP.deepSleep(timeSleep*1000000);
        }
        else
        {
          digitalClockDisplay();
          timeGoingToSleep = timeOfSleep;
          LittleFS.setTimeCallback(sleepingTime);
          deleteFile("/savedTime.txt");
          writeFile("/savedTime.txt", "0");
          Debug.println("going to deepSleep during "+String(timeOfSleep/60)+" minutes");
          ESP.deepSleep(2*3600*1000000); //base : 2h
        }
      }

      //------------------------------------------------------------
      //      GESTION DE NO WIFI CONNECTION : reboot toutes les 2h
      //------------------------------------------------------------
      if(ntpStatusLong >= 720) //720 --> 2x60x6 car check l'heure toutes les 10secs
      {
        deleteFile("/savedTime.txt");
        writeFile("/savedTime.txt", "0");
        Debug.println("restart because noWifi");
        ESP.deepSleep(10);
      }
    }
  }  
}
