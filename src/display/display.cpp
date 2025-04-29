#include "display.h"

// TFT object
TFT_eSPI tft = TFT_eSPI(320, 480);

// Touchscreen object
Adafruit_FT6206 ctp = Adafruit_FT6206();

void displayInit(void) {
    // Initialse display drivers
    tft.begin();
    tft.invertDisplay(true);
    tft.setRotation(0);
    tft.fillScreen(TFT_DARKGREY);

    // Initialise touchscreen driver
    Wire1.setSDA(PIN_TFT_SDA);
    Wire1.setSCL(PIN_TFT_SCL);
    ctp.begin(40, &Wire1);
}