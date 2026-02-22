#include "SdCard.h"

#include <Log.h>

namespace BitGrid {
namespace HAL {

bool SdCard::begin() {
    using namespace Config;

    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    Log::info("SD", "Initialising SD card on CS pin %u", SD_CS);

    if (!SD.begin(SD_CS)) {
        Log::error("SD", "SD.begin() failed. Check card format, wiring, or module.");
        mounted_ = false;
        return false;
    }

    mounted_ = true;
    Log::info("SD", "SD card initialised successfully");
    logCardInfo();
    return true;
}

void SdCard::logCardInfo() const {
    if (!mounted_) {
        Log::warn("SD", "logCardInfo() called but SD not mounted");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Log::error("SD", "No SD card attached or not detected");
        return;
    }

    const char *typeStr = "Unknown";
    if (cardType == CARD_MMC)      typeStr = "MMC";
    else if (cardType == CARD_SD)  typeStr = "SDSC";
    else if (cardType == CARD_SDHC) typeStr = "SDHC/SDXC";

    uint64_t cardSizeMB = SD.cardSize() / (1024ULL * 1024ULL);

    Log::info("SD", "Card type: %s", typeStr);
    Log::info("SD", "Card size: %llu MB", cardSizeMB);
}

void SdCard::logRootDir(uint8_t levels) const {
    if (!mounted_) {
        Log::warn("SD", "Cannot list directory; SD not mounted");
        return;
    }

    Log::info("SD", "Listing root directory /");

    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        Log::error("SD", "Failed to open root directory");
        return;
    }

    std::function<void(File &, uint8_t)> recurse;

    recurse = [&](File &dir, uint8_t depth) {
        File file = dir.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                Log::info("SD", "DIR  %s", file.name());
                if (depth) {
                    File sub = file;
                    recurse(sub, depth - 1);
                }
            } else {
                Log::info("SD", "FILE %s (%u bytes)", file.name(), (unsigned)file.size());
            }
            file = dir.openNextFile();
        }
    };

    recurse(root, levels);
}

} // namespace HAL
} // namespace BitGrid
