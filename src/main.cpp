// BitGrid Player entrypoint — keep this file thin.

#include <Arduino.h>

#include <App.h>

static BitGrid::App app;

void setup() {
  app.begin();
}

void loop() {
  app.tick();
  // Short delay to avoid tight spin until we have a full timing model
  delay(1);
}