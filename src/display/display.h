#pragma once

#include "sys_init.h"

// TFT object
extern TFT_eSPI tft;

// Touchscreen object
extern Adafruit_FT6206 ctp;

void displayInit(void);