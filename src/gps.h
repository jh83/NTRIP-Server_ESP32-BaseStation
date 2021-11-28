#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include "log.h"
#include "settings.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Wire.h> //Needed for I2C to GNSS

extern bool gpsConnected;
extern String gpsInitTime;

extern SFE_UBLOX_GNSS myGNSS;

bool configureGPS();
void initializeGPS();
String getGPSDateTime();
String appendLeadingZero(int input);

#endif
