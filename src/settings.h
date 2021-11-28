#ifndef SETTINGS_H
    #define SETTINGS_H

    #include <Arduino.h>
    #include <Preferences.h>
    #include <ArduinoJson.h>
    #include <SPIFFS.h>

    extern DynamicJsonDocument settings;
    extern DynamicJsonDocument status;

    bool readSettings();
    bool writeSettings(String name, String value);
    void initializeSpiffs();
    String readSpiffsFile(String reqFile);

#endif
