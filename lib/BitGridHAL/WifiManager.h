#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace BitGrid {
namespace HAL {

class WifiManager {
public:
    bool begin(const char* ssid, const char* password);
    bool isConnected() const;
    String getIpAddress() const;

private:
    bool connected_ = false;
};

} // namespace HAL
} // namespace BitGrid
