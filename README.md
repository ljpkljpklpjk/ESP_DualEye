# ESP_DualEye

Waveshare ESP32-S3-DualEye-Touch-LCD-1.28 的 PlatformIO 移植工程。

本项目基于 Arduino framework，使用 LovyanGFX 驱动两块 GC9A01 圆形 LCD，并读取两路 CST816 触摸。当前示例程序会在双屏上显示两只动态眼睛，触摸屏幕时会在对应位置显示触摸点。

## 硬件

- 开发板：Waveshare ESP32-S3-DualEye-Touch-LCD-1.28
- 主控：ESP32-S3R8
- Flash：16MB
- PSRAM：8MB OPI PSRAM
- 显示：2 x 1.28 inch 240x240 GC9A01 SPI LCD
- 触摸：2 x CST816 I2C capacitive touch
- 开发框架：PlatformIO + Arduino

官方文档：

- https://www.waveshare.net/wiki/ESP32-S3-DualEye-Touch-LCD-1.28

## 项目结构

```text
.
├── boards/
│   └── waveshare_esp32_s3_dualeye.json  # PlatformIO 自定义板卡配置
├── include/
│   └── board_config.h                    # Waveshare 板级引脚定义
├── src/
│   └── main.cpp                          # 双屏、触摸和眼睛动画示例
└── platformio.ini                        # PlatformIO 环境配置
```

## 快速开始

安装 PlatformIO 后，在项目根目录执行：

```powershell
pio run
```

连接开发板后烧录：

```powershell
pio run -t upload
```

打开串口监视器：

```powershell
pio device monitor
```

默认串口参数：

- Baud rate：115200
- Upload speed：921600

## 当前功能

- 自定义 PlatformIO 板卡，匹配 Waveshare 这块板的 16MB Flash 和 OPI PSRAM
- 双 GC9A01 LCD 初始化
- 双背光 PWM 控制
- 双 CST816 触摸读取
- 双眼动画测试界面
- 使用 LovyanGFX Sprite 离屏绘制，减少直接清屏重绘造成的闪屏

## 关键引脚

LCD 共用 SPI 总线：

| 功能 | GPIO |
| --- | --- |
| LCD_DOUT / MISO | 40 |
| LCD_CLK / SCLK | 41 |
| LCD_DIN / MOSI | 42 |
| LCD_DC | 45 |

左屏 / LCD1：

| 功能 | GPIO |
| --- | --- |
| CS | 47 |
| RST | 48 |
| BL | 46 |
| TOUCH_SDA | 11 |
| TOUCH_SCL | 10 |
| TOUCH_INT | 5 |
| TOUCH_RST | 4 |

右屏 / LCD2：

| 功能 | GPIO |
| --- | --- |
| CS | 38 |
| RST | 8 |
| BL | 39 |
| TOUCH_SDA | 3 |
| TOUCH_SCL | 2 |
| TOUCH_INT | 7 |
| TOUCH_RST | 6 |

更多引脚见 `include/board_config.h`。

## 闪屏说明

早期版本直接在 LCD 上 `fillScreen()` 后重绘眼睛，能看到明显闪屏。当前版本改为：

1. 每块屏幕创建一个 240x240、16-bit 的 `LGFX_Sprite`
2. 在 Sprite 中完成整帧绘制
3. 通过 `pushSprite()` 一次性推送到对应 LCD
4. 主循环按约 30 FPS 节流

如果仍有轻微闪烁或颜色异常，可以尝试降低 `src/main.cpp` 中 `freq_write`，例如从 `80000000` 改为 `60000000` 或 `40000000`。

## 注意事项

- 本项目优先移植了显示和触摸，SD 卡、音频芯片等外设只保留了板级引脚定义。
- 两块 LCD 共用 SPI 总线，分别使用独立 CS/RST/BL。
- 两路触摸使用不同 I2C 引脚，但 CST816 地址相同，代码中使用 `TwoWire(0)` 和 `TwoWire(1)` 分别访问。
- 如果 PlatformIO 首次编译较慢，是因为需要下载 ESP32 平台包和 LovyanGFX 依赖。
