#include <Arduino.h>
#include <ArduinoJson.h>
#include "log.h"
#include "gps.h"

String logs[logSize];
int prevPosition = 0;

String getLog()
{
  // String built as a JSON
  String resp = "{\"log\":[";

  // Append each log to the string and add a comma since we want it in JSON format
  for (int i = 0; i < logSize; i++)
  {
    if (logs[i].length() != 0)
    {
      resp = resp + logs[i] + ",";
    }
  }

  // Remove last comma
  int lastIndex = resp.length() - 1;
  resp.remove(lastIndex);

  // Append chars to complete the JSON
  resp = resp + "]}";
  return resp;
}

void addToLog(String input)
{

  // Create new log string
  String gpsTime = getGPSDateTime();
  String msg = "[\"" + gpsTime + "\",\"" + input + "\"]";

  // if all positions in array are used, remove the oldest in the array
  // and add the current log to the last position
  if (prevPosition == logSize - 1)
  {
    for (int a = 0; a < (logSize - 1); a++)
    {
      logs[a] = logs[a + 1];
    }
    logs[logSize - 1] = msg;
  }

  // If array isn't full. Add the log to the string array
  else
  {
    for (int i = 0; i < logSize; i++)
    {
      if (logs[i].length() == 0)
      {
        logs[i] = msg;
        prevPosition = i;
        break;
      }
    }
  }
}