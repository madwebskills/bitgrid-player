#include "Log.h"

#include <stdarg.h>

namespace BitGrid {
namespace {
    LogLevel g_level = LogLevel::DEBUG;
}

namespace Log {

void begin(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void setLevel(LogLevel level) {
    g_level = level;
}

LogLevel getLevel() {
    return g_level;
}

static const char *levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default:              return "UNK";
    }
}

void vlogf(LogLevel level, const char *tag, const char *fmt, va_list args) {
    if (static_cast<uint8_t>(level) < BITGRID_LOG_LEVEL) {
        return;
    }

    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    Serial.printf("[%s][%s] %s\n", levelToString(level), tag ? tag : "-", buf);
}

void logf(LogLevel level, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlogf(level, tag, fmt, args);
    va_end(args);
}

} // namespace Log
} // namespace BitGrid
