#ifndef WEB_SERVER_H
    #define WEB_SERVER_H

    #include <Arduino.h>
    #include <MongooseCore.h>
    #include <MongooseHttpServer.h>
    #include "settings.h"
    #include "gps.h"
    #include "ntrip.h"

    void initializeWebServer();

#endif