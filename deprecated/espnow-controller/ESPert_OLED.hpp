#ifndef CMMC_ESPERT_OLED_H
#define CMMC_ESPERT_OLED_H

#include <Arduino.h>
#include "logo.h"
// Include the correct display library
// For a connection via I2C using Wire include
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// For a connection via I2C using brzo_i2c (must be installed) include
// #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Brzo.h"
// For a connection via SPI include
// #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Spi.h"

// Include the UI lib
#include "OLEDDisplayUi.h"

// ****************************************
// OLED class
class ESPert_OLED : public Print
{
  private:
    SSD1306* display;

  public:
    ESPert_OLED();
    void init();
    bool isReady();
    void clear(bool clearImmediately = true);
    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void setColor(uint16_t c);
    void setCursor(int16_t x, int16_t y);
    int16_t getCursorX();
    int16_t getCursorY();
    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color, bool drawImmediately = true);
    void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* bitmap, bool drawImmediately = true);
    void update();
    SSD1306* getDisplay();

    int cursorX;
    int cursorY;

    int charWidth;
    int charHeight;

    const int maxX = 128;
    const int maxY = 64;

#if ARDUINO >= 100
    virtual size_t write(uint8_t);
#else
    virtual void write(uint8_t);
#endif
};

ESPert_OLED::ESPert_OLED() {
  display = NULL;
}

void ESPert_OLED::init() {
  if (!display) {
    display = new SSD1306(0x3c, 4, 5);

#if (SSD1306_LCDHEIGHT != 64)
    //#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

    if (display) {
      // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
      //display->begin(SSD1306_SWITCHCAPVCC, 0x3C, true, ESPERT_PIN_SDA, ESPERT_PIN_SCL);  // initialize with the I2C addr 0x3D (for the 128x64)
      display->init();
      display->flipScreenVertically();

      display->clear();
      drawBitmap(0, 0, logo, 128, 64, WHITE, true);
      // delay(1000);
      // init done

      // Show image buffer on the display hardware.
      // Since the buffer is intialized with an Adafruit splashscreen
      // internally, this will display the splashscreen.
      display->display();

      wdt_disable();
      wdt_enable(WDTO_8S);

      setTextSize(1);
      setTextColor(WHITE);
      setCursor(0, 0);
    }
  }
}

bool ESPert_OLED::isReady() {
  return (display ? true : false);
}

#if ARDUINO >= 100
size_t ESPert_OLED::write(uint8_t c)
#else
void ESPert_OLED::write(uint8_t c)
#endif
{
  char t[2];
  t[0] = c;
  t[1] = 0;

  if (c == 13) {
    update();
    cursorX = 0;
    return 1;
  } else if (c == 10) {
    cursorY += charHeight;
    return 1;
  }
  display->drawString(cursorX, cursorY, String(t));
  cursorX += charWidth;
  if ((cursorX + charWidth) > maxX) {
    cursorX = 0;
    cursorY += charHeight;
  }

#if ARDUINO >= 100
  return 1;
#endif
}

void ESPert_OLED::clear(bool clearImmediately) {
  if (display) {
    display->clear();
    setCursor(0, 0);

    if (clearImmediately) {
      display->display();
    }
  }
}

void ESPert_OLED::setTextSize(uint8_t s) {
  if (display) {
    //display->setTextSize(s);
    charWidth = 6;
    charHeight = 8;
  }
}

void ESPert_OLED::setTextColor(uint16_t c) {
  if (display) {
    display->setColor((OLEDDISPLAY_COLOR)c);
  }
}

void ESPert_OLED::setColor(uint16_t c) {
  if (display) {
    display->setColor((OLEDDISPLAY_COLOR)c);
  }
}

void ESPert_OLED::setCursor(int16_t x, int16_t y) {
  cursorX = x;
  cursorY = y;
}

int16_t ESPert_OLED::getCursorX() {
  return cursorX;
}

int16_t ESPert_OLED::getCursorY() {
  return cursorY;
}

void ESPert_OLED::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color, bool drawImmediately) {
  if (display) {
    display->setColor((OLEDDISPLAY_COLOR)color);
    display->drawXbm(x, y, w, h, (const char*)bitmap);

    if (drawImmediately) {
      display->display();
    }
  }
}

void ESPert_OLED::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* bitmap, bool drawImmediately) {
  if (display) {
    display->drawXbm(x, y, w, h, (const char*)bitmap);

    if (drawImmediately) {
      display->display();
    }
  }
}

void ESPert_OLED::update() {
  if (display) {
    display->display();
  }
}

SSD1306* ESPert_OLED::getDisplay() {
  return display;
}

#endif
