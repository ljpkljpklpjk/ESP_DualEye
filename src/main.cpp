#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <Wire.h>

#include "board_config.h"

class DualEyeDisplay : public lgfx::LGFX_Device {
 public:
  DualEyeDisplay(int cs, int rst, int bl) {
    auto busConfig = bus.config();
    busConfig.spi_host = SPI2_HOST;
    busConfig.spi_mode = 0;
    busConfig.freq_write = 80000000;
    busConfig.freq_read = 16000000;
    busConfig.spi_3wire = false;
    busConfig.use_lock = true;
    busConfig.dma_channel = SPI_DMA_CH_AUTO;
    busConfig.pin_sclk = Board::LcdSclk;
    busConfig.pin_mosi = Board::LcdMosi;
    busConfig.pin_miso = Board::LcdMiso;
    busConfig.pin_dc = Board::LcdDc;
    bus.config(busConfig);

    auto panelConfig = panel.config();
    panelConfig.pin_cs = cs;
    panelConfig.pin_rst = rst;
    panelConfig.pin_busy = -1;
    panelConfig.memory_width = Board::LcdWidth;
    panelConfig.memory_height = Board::LcdHeight;
    panelConfig.panel_width = Board::LcdWidth;
    panelConfig.panel_height = Board::LcdHeight;
    panelConfig.offset_x = 0;
    panelConfig.offset_y = 0;
    panelConfig.offset_rotation = 0;
    panelConfig.dummy_read_pixel = 8;
    panelConfig.dummy_read_bits = 1;
    panelConfig.readable = true;
    panelConfig.invert = true;
    panelConfig.rgb_order = false;
    panelConfig.dlen_16bit = false;
    panelConfig.bus_shared = true;
    panel.config(panelConfig);

    auto lightConfig = light.config();
    lightConfig.pin_bl = bl;
    lightConfig.invert = false;
    lightConfig.freq = 44100;
    lightConfig.pwm_channel = bl == Board::Lcd1Bl ? 0 : 1;
    light.config(lightConfig);

    panel.setBus(&bus);
    panel.setLight(&light);
    setPanel(&panel);
  }

 private:
  lgfx::Bus_SPI bus;
  lgfx::Panel_GC9A01 panel;
  lgfx::Light_PWM light;
};

struct TouchPoint {
  bool touched = false;
  uint16_t x = 0;
  uint16_t y = 0;
};

class Cst816Touch {
 public:
  Cst816Touch(TwoWire& wire, int sda, int scl, int intPin, int rstPin)
      : wire_(wire), sda_(sda), scl_(scl), intPin_(intPin), rstPin_(rstPin) {}

  void begin() {
    pinMode(intPin_, INPUT_PULLUP);
    pinMode(rstPin_, OUTPUT);
    digitalWrite(rstPin_, LOW);
    delay(5);
    digitalWrite(rstPin_, HIGH);
    delay(50);
    wire_.begin(sda_, scl_, 400000);
  }

  TouchPoint read() {
    TouchPoint point;
    uint8_t data[5] = {};
    if (!readRegister(0x02, data, sizeof(data))) {
      return point;
    }

    const uint8_t fingers = data[0] & 0x0F;
    point.touched = fingers > 0;
    if (point.touched) {
      point.x = ((data[1] & 0x0F) << 8) | data[2];
      point.y = ((data[3] & 0x0F) << 8) | data[4];
      point.x = constrain(point.x, 0, Board::LcdWidth - 1);
      point.y = constrain(point.y, 0, Board::LcdHeight - 1);
    }
    return point;
  }

 private:
  bool readRegister(uint8_t reg, uint8_t* data, size_t len) {
    wire_.beginTransmission(Board::Cst816Address);
    wire_.write(reg);
    if (wire_.endTransmission(false) != 0) {
      return false;
    }
    if (wire_.requestFrom(Board::Cst816Address, static_cast<uint8_t>(len)) != len) {
      return false;
    }
    for (size_t i = 0; i < len; ++i) {
      data[i] = wire_.read();
    }
    return true;
  }

  TwoWire& wire_;
  int sda_;
  int scl_;
  int intPin_;
  int rstPin_;
};

DualEyeDisplay leftEye(Board::Lcd1Cs, Board::Lcd1Rst, Board::Lcd1Bl);
DualEyeDisplay rightEye(Board::Lcd2Cs, Board::Lcd2Rst, Board::Lcd2Bl);
TwoWire touchBus1(0);
TwoWire touchBus2(1);
Cst816Touch leftTouch(touchBus1, Board::Touch1Sda, Board::Touch1Scl, Board::Touch1Int, Board::Touch1Rst);
Cst816Touch rightTouch(touchBus2, Board::Touch2Sda, Board::Touch2Scl, Board::Touch2Int, Board::Touch2Rst);
lgfx::LGFX_Sprite leftCanvas(&leftEye);
lgfx::LGFX_Sprite rightCanvas(&rightEye);

void drawEye(lgfx::LGFX_Sprite& canvas, DualEyeDisplay& display, const char* label, uint32_t irisColor, int irisOffsetX, int irisOffsetY, const TouchPoint& touch) {
  canvas.fillScreen(TFT_BLACK);
  canvas.fillCircle(120, 120, 112, TFT_WHITE);
  canvas.drawCircle(120, 120, 112, TFT_DARKGREY);

  const int irisX = 120 + irisOffsetX;
  const int irisY = 120 + irisOffsetY;
  canvas.fillCircle(irisX, irisY, 54, irisColor);
  canvas.fillCircle(irisX, irisY, 25, TFT_BLACK);
  canvas.fillCircle(irisX - 18, irisY - 18, 9, TFT_WHITE);

  canvas.setTextDatum(top_center);
  canvas.setTextColor(TFT_BLACK, TFT_WHITE);
  canvas.drawString(label, 120, 18, &fonts::Font2);

  if (touch.touched) {
    canvas.fillCircle(touch.x, touch.y, 8, TFT_RED);
    canvas.setTextDatum(bottom_center);
    canvas.setTextColor(TFT_RED, TFT_WHITE);
    canvas.drawString("TOUCH", 120, 222, &fonts::Font2);
  }

  canvas.pushSprite(&display, 0, 0);
}

void initCanvas(lgfx::LGFX_Sprite& canvas, const char* name) {
  canvas.setColorDepth(16);
  canvas.setPsram(true);
  if (!canvas.createSprite(Board::LcdWidth, Board::LcdHeight)) {
    Serial.printf("%s canvas allocation failed\n", name);
    while (true) {
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-S3-DualEye-Touch-LCD-1.28 PlatformIO port");

  leftEye.init();
  rightEye.init();
  leftEye.setBrightness(180);
  rightEye.setBrightness(180);
  initCanvas(leftCanvas, "left");
  initCanvas(rightCanvas, "right");

  leftTouch.begin();
  rightTouch.begin();
}

void loop() {
  static uint32_t lastFrame = 0;
  const uint32_t now = millis();
  if (now - lastFrame < 33) {
    delay(1);
    return;
  }
  lastFrame = now;

  const float phase = now / 900.0f;
  const int offsetX = static_cast<int>(sinf(phase) * 28.0f);
  const int offsetY = static_cast<int>(cosf(phase * 0.7f) * 16.0f);

  const TouchPoint leftPoint = leftTouch.read();
  const TouchPoint rightPoint = rightTouch.read();

  drawEye(leftCanvas, leftEye, "LCD1", TFT_SKYBLUE, offsetX, offsetY, leftPoint);
  drawEye(rightCanvas, rightEye, "LCD2", TFT_ORANGE, -offsetX, offsetY, rightPoint);

  if (leftPoint.touched || rightPoint.touched) {
    Serial.printf("touch L:%d,%d,%d R:%d,%d,%d\n",
                  leftPoint.touched, leftPoint.x, leftPoint.y,
                  rightPoint.touched, rightPoint.x, rightPoint.y);
  }
}
