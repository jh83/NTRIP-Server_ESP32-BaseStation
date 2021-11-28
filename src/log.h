#ifndef LOG_H
    #define LOG_H
    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include "gps.h"

    #define logSize 10
    extern String logs[10];

    String getLog();
    void addToLog(String input);

#endif