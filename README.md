# Helicopter VDGS (Arduino Uno R3 + 0.96" SSD1306 OLED)

## Hardware Pinout
| Component | Arduino Uno Pin |
| :--- | :--- |
| **HC-SR04 `TRIG`** | `D9` |
| **HC-SR04 `ECHO`** | `D10` |
| **Green LED** | `D5` (via `330 Ω` resistor to `GND`) |
| **Yellow LED** | `D6` (via `330 Ω` resistor to `GND`) |
| **Red LED** | `D7` (via `330 Ω` resistor to `GND`) |
| **Active Buzzer** | `D8` (`+` to `D8`, `-` to `GND`) |
| **Model Select Button** | `D2` (to `GND`, uses `INPUT_PULLUP`) |
| **0.96" SSD1306 OLED** | `SDA` → `A4`, `SCL` → `A5`, `VCC` → `5V` (or `3.3V`), `GND` → `GND` |

## Quick Deploy & Live Monitor
Double-click `upload_and_monitor.bat` to compile, upload, and view live Serial output (`9600 baud`).
