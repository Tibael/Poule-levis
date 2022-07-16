
/* Set up values for your repository and binary names */
#define GHOTA_USER "Tibael"
#define GHOTA_REPO "Poule-levis"
#define GHOTA_BIN_FILE "Poule-Levis.ino.d1_mini.bin"
#define GHOTA_ACCEPT_PRERELEASE 0

BearSSL::CertStore certStore;

ESPOTAGitHub ESPOTAGitHub(&certStore, GHOTA_USER, GHOTA_REPO, GHOTA_CURRENT_TAG, GHOTA_BIN_FILE,
                            GHOTA_ACCEPT_PRERELEASE);




void handle_upgade() {
  // Initialise Update Code
  // We do this locally so that the memory used is freed when the function
  // exists.

  Serial.println("Checking for update...");
  Serial.println(ESPOTAGitHub.getUpgradeURL());
  if (ESPOTAGitHub.checkUpgrade()) {
    Serial.print("Upgrade found at: ");
    Serial.println(ESPOTAGitHub.getUpgradeURL());
    if (ESPOTAGitHub.doUpgrade()) {
      Serial.println("Upgrade complete.");  // This should never be seen as the device
                                            // should restart on successful upgrade.
    } else {
      Serial.print("Unable to upgrade: ");
      Serial.println(ESPOTAGitHub.getLastError());
    }
  } else {
    Serial.print("Not proceeding to upgrade: ");
    Serial.println(ESPOTAGitHub.getLastError());
  }
}

void setupOTA()
{
  //Arduino OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  //GitHub Update
  //SPIFFS.begin();
  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.print(F("Number of CA certs read: "));
  Serial.println(numCerts);
  if (numCerts == 0) {
    Debug.println(
        F("No certs found. Did you run certs-from-mozill.py and upload the "
          "SPIFFS "
          "directory before running?"));
    return;  // Can't connect to anything w/o certs!
  }

  if (strlen(GHOTA_CURRENT_TAG) == 0) {
    Serial.println("Skipping update check because no version is set");
    do_update_check = false;
    return;
  }
}
