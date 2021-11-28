#include <Arduino.h>
#include "gps.h"
#include "log.h"
#include "settings.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Wire.h> //Needed for I2C to GNSS

SFE_UBLOX_GNSS myGNSS;

bool gpsConnected = false;
String gpsInitTime;

void initializeGPS()
{
  Wire.begin();
  delay(100);
  yield();

  long millisNow = millis();
  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address"));
    addToLog("ERR: GPS - Not detected at default I2C address");
    gpsConnected = false;
  }
  else
  {
    // Configure the GPS module
    gpsConnected = configureGPS();

    while ((millis() - millisNow) < 10000 && gpsConnected == false)
    {
      delay(10);
      yield();
    }
    if (gpsConnected)
    {
      delay(200);
      gpsInitTime = getGPSDateTime();
    }
  }
  //}
}

bool configureGPS()
{
  bool result = false;

  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); //UBX+RTCM3 is not a valid option so we enable all three.
  myGNSS.setNavigationFrequency(1);                                   //Set output in Hz. RTCM rarely benefits from >1Hz.

  //Disable all NMEA sentences
  bool response = true;
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GGA, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_RMC, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GST, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_I2C);
  if (response == false)
  {
    Serial.println(F("Failed to disable NMEA. Freezing..."));
    addToLog("ERR: GPS - Failed to disable NMEA.");
  }
  else
  {
    Serial.println(F("NMEA disabled"));
    addToLog("INF: GPS - NMEA disabled.");
  }
  //Enable necessary RTCM sentences
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1); //Enable message 1005 to output through UART2, message every second
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10); //Enable message every 10 seconds

  if (response == false)
  {
    Serial.println(F("Failed to enable RTCM. Freezing..."));
    addToLog("ERR: GPS - Failed to enable RTCM.");
  }
  else
  {
    Serial.println(F("RTCM sentences enabled"));
    addToLog("INF: GPS - RTCM sentences enabled.");
  }
  //-1280208.308,-4716803.847,4086665.811 is SparkFun HQ so...
  //Units are cm with a high precision extension so -1234.5678 should be called: (-123456, -78)
  //For more infomation see Example12_setStaticPosition
  //Note: If you leave these coordinates in place and setup your antenna *not* at SparkFun, your receiver
  //will be very confused and fail to generate correction data because, well, you aren't at SparkFun...
  //See this tutorial on getting PPP coordinates: https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station/all
  //response &= myGNSS.setStaticPosition(-128020830, -80, -471680384, -70, 408666581, 10); //With high precision 0.1mm parts

  String ecefX = settings["ecefX"];
  String ecefY = settings["ecefY"];
  String ecefZ = settings["ecefZ"];

  double ecefXVal = ecefX.toDouble() * 100;
  double ecefYVal = ecefY.toDouble() * 100;
  double ecefZVal = ecefZ.toDouble() * 100;

  long ecefXWhole = ecefXVal;
  int ecefXRemainder = (ecefXVal - ecefXWhole) * 100;
  long ecefYWhole = ecefYVal;
  int ecefYRemainder = (ecefYVal - ecefYWhole) * 100;
  long ecefZWhole = ecefZVal;
  int ecefZRemainder = (ecefZVal - ecefZWhole) * 100;

  Serial.print("ecefX: ");
  Serial.print(ecefXWhole);
  Serial.print(":::");
  Serial.println(ecefXRemainder);

  Serial.print("ecefY: ");
  Serial.print(ecefYWhole);
  Serial.print(":::");
  Serial.println(ecefYRemainder);

  Serial.print("ecefZ: ");
  Serial.print(ecefZWhole);
  Serial.print(":::");
  Serial.println(ecefZRemainder);

  response &= myGNSS.setStaticPosition(ecefXWhole, ecefXRemainder, ecefYWhole, ecefYRemainder, ecefZWhole, ecefZRemainder); //With high precision 0.1mm parts
  if (response == false)
  {
    Serial.println(F("Failed to enter static position. Freezing..."));
    addToLog("ERR: GPS - Failed to enter static position.");
  }
  else
    Serial.println(F("Static position set"));
  addToLog("INF: GPS - Static position set.");

  //You could instead do a survey-in but it takes much longer to start generating RTCM data. See Example4_BaseWithLCD
  //myGNSS.enableSurveyMode(60, 5.000); //Enable Survey in, 60 seconds, 5.0m

  //Save the current settings to flash and BBR
  if (myGNSS.saveConfiguration() == false)
  {
    Serial.println(F("Module failed to save."));
    addToLog("ERR: GPS - Failed to save settings.");
    result = false;
  }

  Serial.println(F("GPS Module configuration complete"));
  addToLog("INF: GPS - Module configuration complete");

  result = true;

  return result;
}

String getGPSDateTime()
{
  if (myGNSS.getDateValid())
  {
    String YYYY = appendLeadingZero(myGNSS.getYear());
    String MM = appendLeadingZero(myGNSS.getMonth());
    String DD = appendLeadingZero(myGNSS.getDay());
    String hh = appendLeadingZero(myGNSS.getHour());
    String mm = appendLeadingZero(myGNSS.getMinute());
    String ss = appendLeadingZero(myGNSS.getSecond());

    return YYYY + "-" + MM + "-" + DD + " " + hh + ":" + mm + ":" + ss;
  }
  else
  {
    return "--";
  }
}

String appendLeadingZero(int input)
{
  if (input < 10)
  {
    return "0" + String(input);
  }
  return String(input);
}