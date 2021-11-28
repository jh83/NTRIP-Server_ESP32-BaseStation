#ifndef NTRIP_H
    #define NTRIP_H

    #include <Arduino.h>
    #include <ETH.h>
    #include "gps.h"

    extern bool ntripConnected;
    extern uint32_t ntripServerBytesSent;

    bool stopNTRIP();
    bool startNTRIP();
    void handleNTRIP();

#endif