#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <WebServer.h>
#include "WebContent.h"
#include <ArduinoJson.h>
#include "config.h"
#include "NetworkManager.h"
#include "AudioDriver.h"
#include "LedDriver.h"
#include "DisplayDriver.h"

class WebManager {
public:
    WebManager(WatchNetworkManager* networkManager, AudioDriver* audioDriver, LedDriver* ledDriver, DisplayDriver* displayDriver);
    void begin();
    void handle(); // Required for synchronous WebServer

private:
    WebServer server;
    WatchNetworkManager* net;
    AudioDriver* audio;
    LedDriver* led;
    DisplayDriver* display;
    
    void setupRoutes();
};

#endif // WEB_MANAGER_H
