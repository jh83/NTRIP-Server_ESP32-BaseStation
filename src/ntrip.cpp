#include <Arduino.h>
#include <ETH.h>
#include "ntrip.h"
#include "gps.h"

WiFiClient client;

int connectionTimeout = 3000; // MS threshold for timeout when connecting to RKT2go

bool ntripConnected = false;
long lastReport_ms = 0;             //Time of last report of bytes sent
long lastSentRTCM_ms = 0;           //Time of last data pushed to socket
int maxTimeBeforeHangup_ms = 10000; //If we fail to get a complete RTCM frame after 10s, then disconnect from caster
uint32_t ntripServerBytesSent = 0;  //Just a running total

bool stopNTRIP()
{
  Serial.println("Disconnecting NTRIP...");

  client.stop();
  addToLog("INF: NTRIP - Disconneted");
  ntripConnected = false;
  ntripServerBytesSent = 0;
  return true;
}

bool startNTRIP()
{
  const char *casterHost;
  uint16_t casterPort;
  const char *ntrip_sName;
  const char *rtk_mntpnt_pw;
  const char *rtk_mntpnt;

  casterHost = settings["casterHost"];
  casterPort = settings["casterPort"];
  ntrip_sName = settings["ntrip_sName"];
  rtk_mntpnt_pw = settings["rtk_mntpnt_pw"];
  rtk_mntpnt = settings["rtk_mntpnt"];


  if (client.connected() == false)
  {
    Serial.printf("Opening socket to %s\n", casterHost);
    addToLog("INF: NTRIP - Opening socket to " + String(casterHost) + ":" + String(casterPort));

    if (client.connect(casterHost, casterPort, connectionTimeout) == true) //Attempt connection
    {
      Serial.printf("Connected to %s:%d\n", casterHost, casterPort);

      const int SERVER_BUFFER_SIZE = 512;
      char serverBuffer[SERVER_BUFFER_SIZE];

      snprintf(serverBuffer, SERVER_BUFFER_SIZE, "SOURCE %s /%s\r\nSource-Agent: NTRIP %s/%s\r\n\r\n",
               rtk_mntpnt_pw, rtk_mntpnt, ntrip_sName, "App Version 1.0");

      Serial.printf("Sending credentials:\n%s\n", serverBuffer);
      client.write(serverBuffer, strlen(serverBuffer));

      //Wait for response
      unsigned long timeout = millis();
      while (client.available() == 0)
      {
        if (millis() - timeout > connectionTimeout)
        {
          Serial.println(">>> Client Timeout !");
          addToLog("ERR: NTRIP - Failed to connect to RTK2Go: Client Timeout");

          stopNTRIP();
          break;
        }
        delay(100);
        yield();
      }

      //Check reply
      bool connectionSuccess = false;
      char response[512];
      int responseSpot = 0;
      while (client.available())
      {
        response[responseSpot++] = client.read();
        if (strstr(response, "200") > 0) //Look for 'ICY 200 OK'
          connectionSuccess = true;
        if (responseSpot == 512 - 1)
          break;
        yield();
      }
      response[responseSpot] = '\0';

      if (connectionSuccess == false)
      {
        Serial.printf("Failed to connect to RTK2Go: %s", response);
        addToLog("ERR: NTRIP - Failed to connect to RTK2Go: " + String(response));
      }
    } //End attempt to connect
    else
    {
      Serial.println("Connection to host failed");
      //addToLog("ERR: NTRIP - Connection to host failed.");
    }
  }

  delay(100);
  if (client.connected() == true)
  {
    status["ntripConnectedTime"] = getGPSDateTime();
    addToLog("INF: NTRIP - Connected to " + String(casterHost) + ":" + String(casterPort));
  }
  else
  {
    status["ntripConnectedTime"] = 0;
    addToLog("ERR: NTRIP - Connection to host failed.");
    ntripConnected = false;
  }
  return client.connected();
}

void handleNTRIP()
{
  if (client.connected() == true) {

    lastReport_ms = millis();
    lastSentRTCM_ms = millis();

    myGNSS.checkUblox(); //See if new data is available. Process bytes as they come in.

    //Close socket if we don't have new data for 10s
    //RTK2Go will ban your IP address if you abuse it. See http://www.rtk2go.com/how-to-get-your-ip-banned/
    //So let's not leave the socket open/hanging without data
    if (millis() - lastSentRTCM_ms > maxTimeBeforeHangup_ms)
    {
      Serial.println("RTCM timeout. Disconnecting...");
      addToLog("ERR: NTRIP - RTCM timeout. Disconneting...");
      if (stopNTRIP() == true)
      {
        ntripConnected = false;
      }
      return;
    }

    delay(10);

    //Report some statistics every 250
    if (millis() - lastReport_ms > 250)
    {
      lastReport_ms += 250;
      Serial.printf("Total sent: %d\n", ntripServerBytesSent);
    }
  } else {
    addToLog("INF: NTRIP - Disconnected.");
    ntripConnected = false;
  }
}

//This function gets called from the SparkFun u-blox Arduino Library.
//As each RTCM byte comes in you can specify what to do with it
//Useful for passing the RTCM correction data to a radio, Ntrip broadcaster, etc.
void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
{
  if (client.connected() == true)
  {
    client.write(incoming); //Send this byte to socket
    ntripServerBytesSent++;
    lastSentRTCM_ms = millis();
  }
}
