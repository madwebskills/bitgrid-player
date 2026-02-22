// SD card + LED matrix test for ESP32-C3 Super Mini

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <FastLED.h>

// SD wiring:
// CS   -> GPIO7
// MOSI -> GPIO6
// CLK  -> GPIO4
// MISO -> GPIO5

#define SD_SCK   4
#define SD_MISO  5
#define SD_MOSI  6
#define SD_CS    7

// LED matrix wiring:
// DATA -> GPIO10

#define LED_DATA_PIN 10

#define TILE_W 8
#define TILE_H 8
#define TILES_X 3
#define TILES_Y 3

#define WIDTH   (TILE_W * TILES_X)   // 24
#define HEIGHT  (TILE_H * TILES_Y)   // 24
#define NUM_LEDS (WIDTH * HEIGHT)    // 576

CRGB leds[NUM_LEDS];

// Mapping: tiles row-major, pixels row-major, origin top-left
int xy_to_index(int x, int y) {
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 0;

  int tileX = x / TILE_W;
  int tileY = y / TILE_H;
  int localX = x % TILE_W;
  int localY = y % TILE_H;

  int tileIndex = tileY * TILES_X + tileX;
  int localIndex = localY * TILE_W + localX; // NOT serpentine

  return tileIndex * (TILE_W * TILE_H) + localIndex;
}

void setPx(int x, int y, const CRGB &c) {
  leds[xy_to_index(x, y)] = c;
}

void fillAll(const CRGB &c) {
  fill_solid(leds, NUM_LEDS, c);
  FastLED.show();
}

void clearAll() {
  fillAll(CRGB::Black);
}

void runLedSelfTest() {
  Serial.println("Starting LED matrix self-test...");

  // 1) Corners
  clearAll();
  setPx(0, 0, CRGB::Red);                       // TL
  setPx(WIDTH - 1, 0, CRGB::Green);             // TR
  setPx(0, HEIGHT - 1, CRGB::Blue);             // BL
  setPx(WIDTH - 1, HEIGHT - 1, CRGB::White);    // BR
  FastLED.show();
  delay(1500);

  // 2) Simple colour fills
  fillAll(CRGB(32, 0, 0)); delay(800);
  fillAll(CRGB(0, 32, 0)); delay(800);
  fillAll(CRGB(0, 0, 32)); delay(800);

  // 3) Quick checkerboard
  clearAll();
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      setPx(x, y, ((x + y) & 1) ? CRGB::Purple : CRGB::Green);
    }
  }
  FastLED.show();
  delay(1500);

  clearAll();
  Serial.println("LED matrix self-test complete.");
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR  ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void printCardInfo() {
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached or not detected.");
    return;
  }

  Serial.print("Card type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC/SDXC");
  } else {
    Serial.println("Unknown");
  }

  uint64_t cardSize = SD.cardSize() / (1024ULL * 1024ULL);
  Serial.printf("Card size: %llu MB\n", cardSize);
}

bool testReadWrite() {
  const char *path = "/sd_test.txt";

  Serial.println("\n--- Writing test file ---");
  File f = SD.open(path, FILE_WRITE);
  if (!f) {
    Serial.println("Failed to open file for writing.");
    return false;
  }

  f.println("ESP32-C3 SD card test");
  f.println("Line 1: Hello from ESP32-C3!");
  f.println("Line 2: Read/write test OK if you see this.");
  f.printf("Millis at write: %lu\n", (unsigned long)millis());
  f.close();

  Serial.println("Write complete. Re-opening for read...");

  File rf = SD.open(path, FILE_READ);
  if (!rf) {
    Serial.println("Failed to open file for reading.");
    return false;
  }

  Serial.println("--- File contents ---");
  while (rf.available()) {
    Serial.write(rf.read());
  }
  Serial.println("\n--- End of file ---");
  rf.close();

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000); // give USB serial a moment
  Serial.println();
  Serial.println("ESP32-C3 SD card SPI test starting...");

  // Initialise LED matrix
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(32);
  clearAll();

  // Configure SPI with your custom pins
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  Serial.printf("Initializing SD card on CS pin %d...\n", SD_CS);
  // You can lower the SPI clock here if wiring is long/unstable, e.g. 10 MHz
  if (!SD.begin(SD_CS)) {
    Serial.println("SD.begin() failed.");
    Serial.println("Common causes:");
    Serial.println("  - Card not formatted as FAT32 (128GB often exFAT)");
    Serial.println("  - Wiring or power issue");
    Serial.println("  - Incompatible card/module");
    return;
  }

  Serial.println("SD card initialized successfully.\n");
  printCardInfo();

  listDir(SD, "/", 2);

  if (testReadWrite()) {
    Serial.println("\nSD read/write test: SUCCESS");
  } else {
    Serial.println("\nSD read/write test: FAILED");
  }

  // Run LED test after SD test so both subsystems are exercised
  runLedSelfTest();
}

void loop() {
  // Simple heartbeat so you know firmware is still alive
  static uint32_t counter = 0;
  Serial.print("Loop heartbeat (SD test ran at boot) ");
  Serial.println(counter++);
  delay(1000);
}