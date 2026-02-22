#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include <Config.h>

namespace BitGrid {
namespace HAL {

class SdCard {
public:
    bool begin();
    bool isMounted() const { return mounted_; }

    fs::FS &fs() { return SD; }

    void logCardInfo() const;
    void logRootDir(uint8_t levels) const;

private:
    bool mounted_ = false;
};

} // namespace HAL
} // namespace BitGrid
