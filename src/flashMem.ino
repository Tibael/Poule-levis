
void restoreVariable(){
 //adresse mail
    String result = "";
    File this_file = LittleFS.open("/adresseEmail.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("adresseEmail.txt non trouvé");
      WebSerial.println("adresseEmail.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      emailAddress = result;
      Serial.println("adresse Email : "+emailAddress);
      WebSerial.println("adresse Email : "+emailAddress);
    }
 //adresse mail - password
    result = "";
    this_file = LittleFS.open("/emailPassword.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("emailPassword.txt non trouvé");
      WebSerial.println("emailPassword.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      emailPassword = result;
      emailSend = EMailSender(emailAddress.c_str(), emailPassword.c_str());
      Serial.println("adresse Email, mot de passe : "+emailPassword);
      WebSerial.println("adresse Email, mot de passe : "+emailPassword);
    }
 //openAfterSunrise --- TIME AFTER SUNRISE
    result = "";
    this_file = LittleFS.open("/openAfterSunrise.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("openAfterSunrise.txt non trouvé");
      WebSerial.println("openAfterSunrise.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      openAfterSunrise = result.toInt();
      Debug.print("Saved open time after sunrise = ");
      Debug.println(openAfterSunrise);
    }

 //closeAfterSunset --- TIME AFTER SUNSET
    result = "";
    this_file = LittleFS.open("/closeAfterSunset.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("closeAfterSunset.txt non trouvé");
      WebSerial.println("closeAfterSunset.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      closeAfterSunset = result.toInt();
      Debug.print("Saved close time after sunset = ");
      Debug.println(closeAfterSunset);
    }
  
 //timeOfTimeout --- TIME-UP
    result = "";
    this_file = LittleFS.open("/timeUp.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("timeUp.txt non trouvé");
      WebSerial.println("timeUp.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      timeOfTimeout = result.toInt();
      Debug.print("saved time up = ");
      Debug.println(timeOfTimeout);
    }
 
 //timeOfTimeoutDownMinus --- TIME-DOWN
    result = "";
    this_file = LittleFS.open("/timeDown.txt", "r");
    if (!this_file) { // failed to open the file, retrn empty result
      Serial.println("timeDown.txt non trouvé");
      WebSerial.println("timeDown.txt non trouvé");
    }
    else {
      while (this_file.available()) {
          result += (char)this_file.read();
      }
      this_file.close();
      timeOfTimeoutDownMinus = result.toInt();
      Debug.print("saved timeOfTimeoutDownMinus = ");
      Debug.println(timeOfTimeoutDownMinus);
    }
}

bool getLocalTime(struct tm * info, uint32_t ms) {
  uint32_t count = ms / 10;
  time_t now;

  time(&now);
  localtime_r(&now, info);

  if (info->tm_year > (2016 - 1900)) {
    return true;
  }

  while (count--) {
    delay(10);
    time(&now);
    localtime_r(&now, info);
    if (info->tm_year > (2016 - 1900)) {
      return true;
    }
  }
  return false;
}


void listDir(const char * dirname) {
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm * tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}


void readFile(const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Debug.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(const char * path, const char * message) {
  //Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Debug.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    //Debug.println("File written");
  } else {
    Debug.println("Write failed");
  }
  file.close();
}

void appendFile(const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Debug.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Debug.println("Message appended");
  } else {
    Debug.println("Append failed");
  }
  file.close();
}

void renameFile(const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Debug.println("File renamed");
  } else {
    Debug.println("Rename failed");
  }
}

void deleteFile(const char * path) {
  //Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    //Debug.println("File deleted");
  } else {
    Debug.println("Delete failed");
  }
}
