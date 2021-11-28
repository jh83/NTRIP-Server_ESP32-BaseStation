// Credits to sparkfun for making some of the libraries used in this project
// and also for their guidance on how to get started with RTK GPS
#include <Arduino.h>
#include <ETH.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include "web_server.h"
#include "log.h"
#include "gps.h"
#include "settings.h"
#include "ntrip.h"


// NTRIP connect/reconnect interval
const long interval = 60000;
unsigned long previousMillis;

// Function for handling ETH interface events
void EthEvent(WiFiEvent_t event)
{
  Serial.println("ETH");
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START:
    Serial.println("ETH Started");
    //set eth hostname here
    ETH.setHostname(settings["hostname"]);
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    //eth_connected = true;
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    //eth_connected = false;
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    //eth_connected = false;
    break;
  default:
    break;
  }
}

void setup()
{
  status["ntripConnectedMillis"] = 0;

  previousMillis = -interval;

  // Start Serial
  Serial.begin(115200);
  Serial.println("Booting");
  Serial.println("Gettings settings from preferences");

  // Read desired settings from "preferences"
  readSettings();

  // Initialize Spiffs which holds the files
  // required by the web server
  initializeSpiffs();

  delay(10);

// Initialize Ethernet interface
  Serial.println("Connecting LAN");
  ETH.begin();
  WiFi.onEvent(EthEvent);

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(settings["hostname"]);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
                 String type;
                 if (ArduinoOTA.getCommand() == U_FLASH)
                   type = "sketch";
                 else // U_SPIFFS
                   type = "filesystem";

                 // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                 Serial.println("Start updating " + type);
               })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
                 Serial.printf("Error[%u]: ", error);
                 if (error == OTA_AUTH_ERROR)
                   Serial.println("Auth Failed");
                 else if (error == OTA_BEGIN_ERROR)
                   Serial.println("Begin Failed");
                 else if (error == OTA_CONNECT_ERROR)
                   Serial.println("Connect Failed");
                 else if (error == OTA_RECEIVE_ERROR)
                   Serial.println("Receive Failed");
                 else if (error == OTA_END_ERROR)
                   Serial.println("End Failed");
               });

  ArduinoOTA.begin();

  // Initialize WebServer
  initializeWebServer();

  // Initialize GPS
  initializeGPS();
}

void loop()
{

  // Handle Mongoose web server
  Mongoose.poll(1000);

  // Handle OTA
  ArduinoOTA.handle();

  // Handle NTRIP connect on intervall
  // If connected, then handle messages in every loop
  unsigned long currentMillis = millis();
  if (gpsConnected == true && ntripConnected == false && settings["connectNtrip"] == true && currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    Serial.println("startNtrip");
    ntripConnected = startNTRIP();
  }
  else if (settings["connectNtrip"] == false && ntripConnected == true)
  {
    stopNTRIP();
    delay(10);
    Serial.println("disconnectNtrip");
  }
  else if (ntripConnected == true)
  {
    handleNTRIP();
    delay(10);
  }

  yield();
}