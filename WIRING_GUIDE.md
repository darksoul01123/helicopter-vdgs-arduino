# Arduino Uno R3 — Helicopter VDGS Wiring Guide

Exact breadboard wiring for [helicopter-vdgs-arduino.ino](file:///d:/app%20dev/helicopter-vdgs-arduino/helicopter-vdgs-arduino.ino).

---

## Schematic Overview

```mermaid
flowchart LR
    subgraph UNO["Arduino Uno R3"]
        P5V["5V"]
        PGND["GND"]
        D2["D2"]
        D5["D5"]
        D6["D6"]
        D7["D7"]
        D8["D8"]
        D9["D9"]
        D10["D10"]
        A4["A4 (SDA)"]
        A5["A5 (SCL)"]
    end

    subgraph RAILS["Breadboard Power Rails"]
        R5V["+ 5V Rail"]
        RGND["- GND Rail"]
    end

    P5V --> R5V
    PGND --> RGND

    subgraph US["HC-SR04 Ultrasonic"]
        UVCC["VCC"]
        UTRIG["TRIG"]
        UECHO["ECHO"]
        UGND["GND"]
    end
    R5V --> UVCC
    D9 --> UTRIG
    D10 --> UECHO
    UGND --> RGND

    subgraph LEDS["LEDs (330 ohm resistors)"]
        LG["Green LED"]
        LY["Yellow LED"]
        LR["Red LED"]
    end
    D5 -->|"330 Ω"| LG --> RGND
    D6 -->|"330 Ω"| LY --> RGND
    D7 -->|"330 Ω"| LR --> RGND

    subgraph BUZ["Active Buzzer"]
        BP["+"]
        BN["-"]
    end
    D8 --> BP
    BN --> RGND

    subgraph OLED["0.96 in SSD1306 OLED"]
        OVCC["VCC"]
        OGND["GND"]
        OSDA["SDA"]
        OSCL["SCL"]
    end
    R5V --> OVCC
    OGND --> RGND
    A4 --> OSDA
    A5 --> OSCL

    subgraph BTN["Push Button (Optional)"]
        B1["Pin 1"]
        B2["Pin 2"]
    end
    D2 --> B1
    B2 --> RGND
```

---

## Step-by-Step Breadboard Checklist (Zero Mistakes)

### 1. Power Rails First
- **Arduino `5V`** → Breadboard **`+` rail**
- **Arduino `GND`** → Breadboard **`-` rail**
- *(Keep all `+` and `-` wires on the **same half** of the breadboard rails in case your breadboard has a split in the middle.)*

### 2. HC-SR04 Ultrasonic Sensor (4 pins)
Plug into **4 separate 5-hole rows** on the breadboard:
- **`VCC`** → **`+` rail (`5V`)**
- **`TRIG`** → **Arduino `D9`**
- **`ECHO`** → **Arduino `D10`**
- **`GND`** → **`-` rail (`GND`)**

### 3. Three LEDs (with `330 Ω` Resistors)
For **each** LED: **longer leg = `+` (anode)**, **shorter leg / flat edge = `-` (cathode)**.
- **Green LED:** `D5` → `330 Ω` resistor → **Green `+`**, **Green `-`** → **`-` rail (`GND`)**
- **Yellow LED:** `D6` → `330 Ω` resistor → **Yellow `+`**, **Yellow `-`** → **`-` rail (`GND`)**
- **Red LED:** `D7` → `330 Ω` resistor → **Red `+`**, **Red `-`** → **`-` rail (`GND`)**

### 4. Active Buzzer
- **`+` (longer leg / `+` mark)** → **Arduino `D8`**
- **`-` (shorter leg)** → **`-` rail (`GND`)**

### 5. 0.96" SSD1306 I2C OLED (4 pins)
- **`VCC`** → **`+` rail (`5V`)**
- **`GND`** → **`-` rail (`GND`)**
- **`SDA`** → **Arduino `A4`**
- **`SCL`** → **Arduino `A5`**

### 6. Model Select Push Button *(Optional)*
- **One leg** → **Arduino `D2`**
- **Opposite leg** → **`-` rail (`GND`)**
