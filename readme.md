# ESP32-C3 Motor Driver

BLE-enabled stepper motor control system with Flask web interface.

## Hardware Connections

| ESP32-C3 Pin | Motor Driver Pin |
|--------------|------------------|
| GPIO12       | DIR              |
| GPIO14       | STEP             |

## Installation

### Firmware
```bash
# Upload BLClient.ino to ESP32-C3 via Arduino IDE
```

### Flask Server
```bash
pip install flask
python main.py
# Access at http://localhost:5000
```

## Usage

Connect to BLE device `LabPumpController` and send JSON commands:

**Run motor:**
```json
{"dir": 1, "speed": 500, "time": 10000}
```

**Stop motor:**
```json
{"stop": true}
```

## Configuration

Set `DEBUG_MODE` in firmware:
- `true` - Testing mode (no motor)
- `false` - Production mode
