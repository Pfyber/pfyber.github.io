#define DIR_PIN 12
#define STEP_PIN 14
#define DEBUG_MODE false  // Set to false to disable motor logic for testing

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

#if DEBUG_MODE
  #include <AccelStepper.h>
  AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
#endif

BLECharacteristic* pControlCharacteristic;
BLECharacteristic* pStatusCharacteristic;
BLEServer* pServer;
bool pumpRunning = false;
unsigned long pumpStartTime = 0;
unsigned long pumpRunTime = 0;
int lastSpeed = 0;
bool clientConnected = false;

#define SERVICE_UUID "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CONTROL_CHAR_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"
#define STATUS_CHAR_UUID "19b10003-e8f2-537e-4f6c-d104768a1214"

class ControlCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String value = pChar->getValue();
    StaticJsonDocument<128> doc;
    DeserializationError error = deserializeJson(doc, value.c_str());

    if (error) {
      Serial.println("JSON parse error");
      return;
    }

    if (doc["stop"] == true) {
      pumpRunning = false;
      #if DEBUG_MODE
        stepper.setSpeed(0);
      #endif
      Serial.println("Received stop command");
      return;
    }

    int direction = doc["dir"];
    int speed = doc["speed"];
    int duration = doc["time"];

    #if DEBUG_MODE
      digitalWrite(DIR_PIN, direction ? HIGH : LOW);
      stepper.setMaxSpeed(speed);
      stepper.setSpeed(speed);
    #endif

    lastSpeed = speed;
    pumpRunTime = duration;
    pumpStartTime = millis();
    pumpRunning = true;

    Serial.printf("Start pump: dir=%d speed=%d time=%d\n", direction, speed, duration);
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    clientConnected = true;
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer* pServer) override {
    clientConnected = false;
    Serial.println("Client disconnected");
    delay(100);
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  #if DEBUG_MODE
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    stepper.setMaxSpeed(1000);
    stepper.setAcceleration(1000);
  #endif

  BLEDevice::init("LabPumpController");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pControlCharacteristic = pService->createCharacteristic(
    CONTROL_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);
  pControlCharacteristic->setCallbacks(new ControlCallbacks());
  pControlCharacteristic->addDescriptor(new BLE2902());

  pStatusCharacteristic = pService->createCharacteristic(
    STATUS_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pStatusCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinInterval(0x20);
  pAdvertising->setMaxInterval(0x30);
  BLEDevice::startAdvertising();

  Serial.println("BLE pump ready for pharmacy lab");
}

void loop() {
  static unsigned long lastUpdate = 0;
  if (pumpRunning) {
    unsigned long now = millis();
    #if DEBUG_MODE
      if (lastSpeed > 0) stepper.runSpeed();
    #endif

    if (now - lastUpdate > 1000) {
      lastUpdate = now;
      unsigned long elapsed = now - pumpStartTime;
      unsigned long remaining = (elapsed < pumpRunTime) ? (pumpRunTime - elapsed) : 0;

      if (clientConnected) {
        StaticJsonDocument<64> statusDoc;
        statusDoc["running"] = true;
        statusDoc["remaining"] = remaining / 1000;
        statusDoc["speed"] = lastSpeed;
        char buffer[64];
        serializeJson(statusDoc, buffer);
        pStatusCharacteristic->setValue(buffer);
        pStatusCharacteristic->notify();
      }
    }

    if (millis() - pumpStartTime >= pumpRunTime) {
      Serial.println("Finished dispensing");
      pumpRunning = false;
      #if DEBUG_MODE
        stepper.setSpeed(0);
      #endif
    }
  }
  delay(10);
}