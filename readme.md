## ESP32-C3 Motor Driver Connection Guide

This guide explains how to connect your ESP32-C3 board to a stepper motor driver using the provided firmware (`BLClient.ino`).

### Hardware Connections

1. **DIR_PIN (Direction):**
	- Connect ESP32-C3 pin **GPIO12** to the **DIR** (direction) input on your stepper motor driver.

2. **STEP_PIN (Step):**
	- Connect ESP32-C3 pin **GPIO14** to the **STEP** input on your stepper motor driver.

3. **Power:**
	- Make sure your motor driver and ESP32-C3 are powered according to their specifications.

### Firmware Overview

- The firmware uses BLE (Bluetooth Low Energy) to receive commands from a client (e.g., a phone or PC).
- Commands control the motor direction, speed, and run time.
- The BLE device name is `LabPumpController`.

### How to Use

1. **Flash the ESP32-C3:**
	- Upload the `BLClient.ino` firmware to your ESP32-C3 using Arduino IDE .

2. **Connect the Pins:**
	- Wire GPIO12 to DIR on the driver.
	- Wire GPIO14 to STEP on the driver.

3. **Connect via BLE:**
	- Use a BLE client app to connect to `LabPumpController`.
	- Send JSON commands to control the motor:
	  - Example: `{"dir":1, "speed":500, "time":10000}`
	  - To stop: `{"stop":true}`

### Notes

- The firmware is set up for testing with DEBUG_MODE. For real motor control, set `DEBUG_MODE` to `false`.

---

