#pragma once

#include <Arduino.h>

namespace Board {
constexpr uint16_t LcdWidth = 240;
constexpr uint16_t LcdHeight = 240;

// Shared GC9A01 SPI bus.
constexpr int LcdMiso = 40;  // LCD_DOUT
constexpr int LcdSclk = 41;  // LCD_CLK
constexpr int LcdMosi = 42;  // LCD_DIN
constexpr int LcdDc = 45;

// Left / LCD1.
constexpr int Lcd1Cs = 47;
constexpr int Lcd1Rst = 48;
constexpr int Lcd1Bl = 46;
constexpr int Touch1Sda = 11;
constexpr int Touch1Scl = 10;
constexpr int Touch1Int = 5;
constexpr int Touch1Rst = 4;

// Right / LCD2.
constexpr int Lcd2Cs = 38;
constexpr int Lcd2Rst = 8;
constexpr int Lcd2Bl = 39;
constexpr int Touch2Sda = 3;
constexpr int Touch2Scl = 2;
constexpr int Touch2Int = 7;
constexpr int Touch2Rst = 6;

// TF card, wired in SPI mode. CS is not connected on this board.
constexpr int SdMiso = 18;
constexpr int SdMosi = 21;
constexpr int SdSclk = 17;

// Shared audio control/I2S pins from the Waveshare schematic table.
constexpr int AudioI2cSda = 11;
constexpr int AudioI2cScl = 10;
constexpr int AudioMclk = 12;
constexpr int AudioSclk = 13;
constexpr int AudioLrck = 14;
constexpr int SpeakerDataIn = 16;  // ES8311 I2S_DSDIN
constexpr int MicDataOut = 15;     // ES7210 I2S_ASDOUT

constexpr uint8_t Cst816Address = 0x15;
}  // namespace Board
