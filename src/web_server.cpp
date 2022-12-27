#include <Arduino.h>

#include <MongooseCore.h>
#include <MongooseHttpServer.h>
#include "settings.h"
#include "gps.h"
#include "ntrip.h"

// HTTP Related
MongooseHttpServer server;

static void notFound(MongooseHttpServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

// Start web server
void initializeWebServer()
{
    Serial.println("Starting webserver");
    Mongoose.begin();
    server.begin(80);

    server.on("/$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { request->send(200, "text/html", readSpiffsFile("/index.html")); });
    server.on("/settings.html$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { request->send(200, "text/html", readSpiffsFile("/settings.html")); });
    server.on("/js/osm.js$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { request->send(200, "application/javascript", readSpiffsFile("/js/osm.js")); });
    server.on("/js/app.js$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { request->send(200, "application/javascript", readSpiffsFile("/js/app.js")); });

    server.on("/restart$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { 
                //String message = "<!DOCTYPE html><html><head><title> Old Page</ title><meta charset = 'UTF-8' /><meta http - equiv = 'refresh' content = '3; URL=https://www.hubspot.com/' /> </ head><body><p> This page has been moved.If you are not redirected within 3 seconds, click<a href = 'https://www.hubspot.com/'> here</ a> to go to the HubSpot homepage.</ p></ body></ html>"; 
                request->send(200, "text/html", "message");
                ESP.restart(); });

    server.on("/settings$", HTTP_GET, [](MongooseHttpServerRequest *request)
              {
                  String message;
                  serializeJson(settings, message);
                  request->send(200, "text/plain", message); });
    server.on("/log$", HTTP_GET, [](MongooseHttpServerRequest *request)
              { request->send(200, "text/plain", getLog()); });

    server.on("/status$", HTTP_GET, [](MongooseHttpServerRequest *request)
              {
                  
                  int32_t latitude = myGNSS.getHighResLatitude();
                  int8_t latitudeHp = myGNSS.getHighResLatitudeHp();
                  int32_t longitude = myGNSS.getHighResLongitude();
                  int8_t longitudeHp = myGNSS.getHighResLongitudeHp();

                  // Defines storage for the lat and lon as double
                  double d_lat; // latitude
                  double d_lon; // longitude

                  // Assemble the high precision latitude and longitude
                  d_lat = ((double)latitude) / 10000000.0;       // Convert latitude from degrees * 10^-7 to degrees
                  d_lat += ((double)latitudeHp) / 1000000000.0;  // Now add the high resolution component (degrees * 10^-9 )
                  d_lon = ((double)longitude) / 10000000.0;      // Convert longitude from degrees * 10^-7 to degrees
                  d_lon += ((double)longitudeHp) / 1000000000.0; // Now add the high resolution component (degrees * 10^-9 )

                  //status["bootMillis"] = millis();
                  status["gpsInitTime"] = gpsInitTime;
                  status["gpsSiv"] = myGNSS.getSIV();
                  status["gpsLatitude"] = serialized(String(d_lat, 9));
                  status["gpsLongitude"] = serialized(String(d_lon, 9));
                  status["gpsConnected"] = gpsConnected;
                  status["ntripConnected"] = ntripConnected;
                  status["ntripServerKBSent"] = ntripServerBytesSent / 1024 / 1024; // to MB

                  String message;
                  serializeJson(status, message);
                  request->send(200, "text/plain", message); });

    server.on("/applySettings$", HTTP_GET, [](MongooseHttpServerRequest *request)
              {
                  String inputMessage;
                  inputMessage = request->message().c_str();

                  int startFrom = inputMessage.indexOf("?") + 1;
                  int endAt = inputMessage.indexOf(" ", startFrom);

                  String substring = inputMessage.substring(startFrom, endAt);

                  String currKey;
                  String currValue;
                  bool isKey = true;

                  for (int i = 0; i <= substring.length(); i++)
                  {
                      char currChar = substring[i];
                      if (currChar == '&' || i == substring.length())
                      {
                          writeSettings(currKey, currValue);
                          currKey = "";
                          currValue = "";
                          isKey = true;
                      }
                      else if (currChar == '=')
                      {
                          isKey = false;
                      }
                      else
                      {
                          if (isKey)
                          {
                              currKey = currKey + currChar;
                          }
                          else
                          {
                              currValue = currValue + currChar;
                          }
                      }
                  }

                  delay(100);

                  request->send(200, "text/plain", inputMessage);

                  ESP.restart(); });

    server.onNotFound(notFound);
}