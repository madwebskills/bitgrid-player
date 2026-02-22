#include "WifiManager.h"

#include <Log.h>

namespace BitGrid {
namespace HAL {

static const char* TAG = "WiFi";

bool WifiManager::begin(const char* ssid, const char* password) {
    Log::info(TAG, "Connecting to WiFi SSID: %s", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // Wait up to 10 seconds for connection
    uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMs < 10000) {
        delay(100);
    }

    connected_ = (WiFi.status() == WL_CONNECTED);

    if (connected_) {
        Log::info(TAG, "Connected! IP address: %s", WiFi.localIP().toString().c_str());
        Log::info(TAG, "Access file manager at: http://%s/", WiFi.localIP().toString().c_str());
    } else {
        Log::error(TAG, "Connection failed. Status: %d", WiFi.status());
    }

    return connected_;
}

bool WifiManager::isConnected() const {
    return connected_ && WiFi.status() == WL_CONNECTED;
}

String WifiManager::getIpAddress() const {
    return WiFi.localIP().toString();
}

} // namespace HAL
} // namespace BitGrid
