#pragma once

#include <Arduino.h>

namespace BitGrid {

enum class LogLevel : uint8_t {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

// Compile-time minimum log level (can be overridden via build_flags)
// Use simple integers for preprocessor compatibility: 0=DEBUG, 1=INFO, 2=WARN, 3=ERROR
#ifndef BITGRID_LOG_LEVEL
#define BITGRID_LOG_LEVEL 0
#endif

namespace Log {

void begin(unsigned long baudRate);
void setLevel(LogLevel level);
LogLevel getLevel();

void vlogf(LogLevel level, const char *tag, const char *fmt, va_list args);
void logf(LogLevel level, const char *tag, const char *fmt, ...);

inline void debug(const char *tag, const char *fmt, ...) {
#if BITGRID_LOG_LEVEL <= 0
    va_list args;
    va_start(args, fmt);
    vlogf(LogLevel::DEBUG, tag, fmt, args);
    va_end(args);
#endif
}

inline void info(const char *tag, const char *fmt, ...) {
#if BITGRID_LOG_LEVEL <= 1
    va_list args;
    va_start(args, fmt);
    vlogf(LogLevel::INFO, tag, fmt, args);
    va_end(args);
#endif
}

inline void warn(const char *tag, const char *fmt, ...) {
#if BITGRID_LOG_LEVEL <= 2
    va_list args;
    va_start(args, fmt);
    vlogf(LogLevel::WARN, tag, fmt, args);
    va_end(args);
#endif
}

inline void error(const char *tag, const char *fmt, ...) {
#if BITGRID_LOG_LEVEL <= 3
    va_list args;
    va_start(args, fmt);
    vlogf(LogLevel::ERROR, tag, fmt, args);
    va_end(args);
#endif
}

} // namespace Log
} // namespace BitGrid
