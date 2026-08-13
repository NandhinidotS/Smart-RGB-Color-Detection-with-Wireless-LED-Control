# Smart RGB Color Detection with Wireless LED Control

## 📌 Project Overview

**Smart RGB Color Detection with Wireless LED Control** is an Arduino UNO-based embedded project that combines **automatic RGB color detection** with **wireless LED control**.

The system uses a **TCS3200 color sensor** to detect Red, Green, and Blue objects and activates the corresponding LED. An **HC-05 Bluetooth module** allows manual LED control from a smartphone.

The project also supports **RGB calibration and EEPROM storage** for more reliable color detection across restarts.

## ✨ Key Features

* TCS3200-based RGB color detection
* Automatic color-based LED indication
* HC-05 Bluetooth wireless control
* Smartphone-based manual control
* RGB calibration with 10-sample averaging
* Normalized RGB processing
* Distance-based color classification
* Unknown-color detection using thresholds
* EEPROM-based calibration storage
* Automatic and Manual operating modes
* Uppercase/lowercase Bluetooth commands

## 🧰 Hardware

| Component    | Purpose                 |
| ------------ | ----------------------- |
| Arduino UNO  | Main controller         |
| TCS3200      | RGB color detection     |
| HC-05        | Bluetooth communication |
| Red LED      | Red indication          |
| Green LED    | Green indication        |
| White LED    | Blue indication         |
| Resistors    | LED current limiting    |
| Jumper Wires | Connections             |

## 🔌 Pin Configuration

### TCS3200

| Pin | Arduino UNO |
| --- | ----------- |
| S0  | D4          |
| S1  | D5          |
| S2  | D6          |
| S3  | D7          |
| OUT | D8          |

### HC-05

| Pin | Arduino UNO             |
| --- | ----------------------- |
| TX  | D10 (SoftwareSerial RX) |
| RX  | D11 (SoftwareSerial TX) |

### LEDs

| LED                    | Pin |
| ---------------------- | --- |
| Red                    | D12 |
| Green                  | D13 |
| White / Blue Indicator | D3  |

> The white LED is intentionally used as the Blue color indicator.

## 🔄 Working

```text
              ┌──────────────────┐
              │  TCS3200 Sensor  │
              └────────┬─────────┘
                       ↓
                RGB Measurement
                       ↓
               RGB Processing
                       ↓
             Color Classification
                       ↓
                  LED Output

Smartphone → HC-05 → Arduino UNO → Manual LED Control
```

The TCS3200 generates a frequency-based output for each RGB component. The Arduino measures the pulse duration using `pulseIn()`, converts the readings into RGB values, normalizes them, and compares them with calibrated reference values.

The closest RGB reference is selected using **Euclidean distance**:

```text
Distance = √[(R₁-R₂)² + (G₁-G₂)² + (B₁-B₂)²]
```

Threshold and separation checks are used to reject uncertain readings as **UNKNOWN COLOR**.

## 🎨 Color Detection

| Detected Color | LED       |
| -------------- | --------- |
| Red            | Red LED   |
| Green          | Green LED |
| Blue           | White LED |

## 📱 Bluetooth Control

The HC-05 receives commands from a smartphone Bluetooth Controller application.

| Command   | Function              |
| --------- | --------------------- |
| `R` / `r` | Red LED ON            |
| `G` / `g` | Green LED ON          |
| `B` / `b` | White LED ON          |
| `A` / `a` | All LEDs ON           |
| `O` / `o` | All LEDs OFF          |
| `C` / `c` | Return to Sensor Mode |

`R`, `G`, `B`, `A`, and `O` activate **Manual Mode**.
`C` returns the system to **Automatic Sensor Mode**.

## 🧠 Calibration & EEPROM

The system calibrates Red, Green, and Blue reference values using **10 samples per color** and stores the averaged values in the Arduino UNO's EEPROM.

```text
Arduino Startup
      ↓
Check EEPROM
   ↙       ↘
Valid      Invalid
  ↓           ↓
Load       Calibrate
Values
  ↓
Sensor Mode
```

A predefined magic value is used to verify whether valid calibration data exists.

## 💻 Technologies & Concepts

* Arduino UNO
* Embedded C / Arduino C++
* GPIO
* Sensor interfacing
* TCS3200 frequency measurement
* `pulseIn()`
* UART communication
* SoftwareSerial
* HC-05 Bluetooth
* EEPROM
* Sensor calibration
* Multi-sample averaging
* RGB normalization
* Euclidean distance
* Threshold-based classification
* `millis()` timing
* State/mode management

### Libraries

```cpp
#include <EEPROM.h>
#include <SoftwareSerial.h>
```

## 🚀 Future Improvements

* LCD/OLED display for RGB values
* Support for more colors
* Automatic LED brightness control
* Better performance under different lighting conditions
* Real-time RGB data logging
* Dedicated mobile application
* ESP32-based Bluetooth/Wi-Fi connectivity
* Buzzer/audio indication

## 🎥 Project Demo

[▶️ Watch Project Demonstration Video](https://drive.google.com/file/d/1qvlauQ8naApLqLxaisIw_zz4-MCsXDGI/view?usp=drivesdk)

## 🏁 Conclusion

This project demonstrates practical implementation of **RGB color sensing, sensor calibration, data processing, EEPROM memory, UART/Bluetooth communication, and LED control** using Arduino UNO.

It combines **automatic color detection** and **wireless manual control** into a single embedded system.
