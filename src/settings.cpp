#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "settings.h"

DynamicJsonDocument settings(1024);
DynamicJsonDocument status(1024);

Preferences preferences;

bool readSettings()
{
  preferences.begin("settings", false);
  settings["casterPort"] = preferences.getUShort("casterPort", 0);
  settings["casterHost"] = preferences.getString("casterHost", "");
  settings["hostname"] = preferences.getString("hostname", "");
  settings["ntrip_sName"] = preferences.getString("ntrip_sName", "");
  settings["rtk_mntpnt_pw"] = preferences.getString("rtk_mntpnt_pw", "");
  settings["rtk_mntpnt"] = preferences.getString("rtk_mntpnt", "");
  settings["connectNtrip"] = preferences.getBool("connectNtrip", 0);
  settings["ecefX"] = preferences.getString("ecefX", "0");
  settings["ecefY"] = preferences.getString("ecefY", "0");
  settings["ecefZ"] = preferences.getString("ecefZ", "0");

  preferences.end();

  Serial.println("Retreived settings:");

  // loop thru JSON document
  JsonObject root = settings.as<JsonObject>();
  for (JsonPair kv : root)
  {
    Serial.print(kv.key().c_str());
    Serial.print(": ");
    serializeJson(settings[kv.key()], Serial);
    Serial.println();
  }

  return true;
}

bool writeSettings(String name, String value)
{

  Serial.println("name: " + name);
  Serial.println("value: " + value);

  preferences.begin("settings", false);

  if (name == "casterPort")
  {
    preferences.putUShort("casterPort", value.toInt());
  }
  else if (name == "connectNtrip")
  {
    preferences.putBool("connectNtrip", value.toInt());
  }
  else
  {
    preferences.putString(name.c_str(), value);
  }

  preferences.end();
  return true;
}

void initializeSpiffs()
{
 
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else
  {
    Serial.println("Mounted SPIFFS");
  }
}

String readSpiffsFile(String reqFile)
{
  String fileContent;

  File file = SPIFFS.open(reqFile);
  if (!file)
  {
    Serial.println("Failed to open file!");
    return "";
  }

  while (file.available())
  {
    fileContent = file.readString();
  }
  file.close();

  return fileContent;
}
