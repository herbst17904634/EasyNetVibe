#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Haptic_Driver.h>
#include <esp32-hal-ledc.h>
#include <M5StickCPlus2.h>
#include <M5GFX.h>

Haptic_Driver hapDrive;
int hapDriveEvent = 0;

void vibrate(int value) {
  hapDrive.setVibrate(value * 127 / 20);
}

const int COUNT_LOW = (((1.0 / 20) * 65536) + 0.5);
const int COUNT_HIGH = (((2.0 / 20) * 65536) + 0.5);
const int TIMER_WIDTH = 16;

BLEServer* pServer = nullptr;
BLECharacteristic* pTxCharacteristic = nullptr;
BLECharacteristic* pRxCharacteristic = nullptr;
String bleAddress = "30c6f743754e";  // CONFIGURATION: < Use the real device BLE address here.
bool deviceConnected = false;
bool oldDeviceConnected = false;
int vibration = 0;

// https://docs.buttplug.io/docs/stpihkal/protocols/lovense
const auto SERVICE_UUID = "53300001-0023-4bd4-bbd5-a6920e4c5653";
const auto CHARACTERISTIC_RX_UUID = "53300002-0023-4bd4-bbd5-a6920e4c5653";
const auto CHARACTERISTIC_TX_UUID = "53300003-0023-4bd4-bbd5-a6920e4c5653";

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  uint8_t messageBuf[64];
  void response(String str) {
    int len = str.length();
    memmove(messageBuf, str.c_str(), len);
    pTxCharacteristic->setValue(messageBuf, len);
    pTxCharacteristic->notify();
    Serial.print("TX: ");
    Serial.write(messageBuf, len);
    Serial.println();
  }
  void onWrite(BLECharacteristic* pCharacteristic) {
    assert(pCharacteristic == pRxCharacteristic);
    std::string rxValue = pRxCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.print("RX: ");
      Serial.println(rxValue.c_str());
    }

    // https://docs.buttplug.io/docs/stpihkal/protocols/lovense/
    // P: Edge - Vibrate1, Vibrate2
    // Z: Hush - Vibrate
    if (rxValue == "DeviceType;") {
      response("Z:ED:c857339ba2a6;");
    } else if (rxValue == "Battery;") {
      auto batteryLevel = StickCP2.Power.getBatteryLevel();
      auto batteryLevelStr = String(batteryLevel) + ";";
      response(batteryLevelStr);
    } else if (rxValue == "PowerOff;") {
      response("OK;");
    } else if (rxValue == "RotateChange;") {
      response("OK;");
    } else if (rxValue.rfind("Status:", 0) == 0) {
      response("2;");
    } else if (rxValue.rfind("Vibrate:", 0) == 0) {
      vibration = std::atoi(rxValue.substr(8).c_str());
      vibrate(vibration);
      response("OK;");
    } else if (rxValue.rfind("Vibrate1:", 0) == 0) {
      vibration = std::atoi(rxValue.substr(9).c_str());
      vibrate(vibration);
      response("OK;");
    } else if (rxValue.rfind("Vibrate2:", 0) == 0) {
      vibration = std::atoi(rxValue.substr(9).c_str());
      int count = map(vibration, 0, 20, COUNT_HIGH, COUNT_LOW);
      ledcWrite(1, count);
      response("OK;");
    } else {
      // Unknown request
      response("ERR;");
    }
  }
};

M5Canvas canvas(&StickCP2.Display);

void drawBarGraph(const int x, const int y, const int graphPosOffset, const int level, const int maxLevel, const char* label) {
  const auto barMaxWidth = canvas.width() - graphPosOffset;
  const auto padding = 6;
  const auto barHeight = canvas.fontHeight() - padding;
  const auto barWidth = map(level, 0, maxLevel, 0, barMaxWidth);

  canvas.setCursor(x, y);
  canvas.printf("%s: %3d", label, level);

  canvas.fillRect(x + graphPosOffset, y + padding / 2, barMaxWidth, barHeight, TFT_DARKGREY);
  canvas.fillRect(x + graphPosOffset, y + padding / 2, barWidth, barHeight, TFT_MAGENTA);
}

void showValues() {
  const auto level = StickCP2.Power.getBatteryLevel();

  canvas.fillSprite(TFT_BLACK);
  canvas.setCursor(0, 0);
  canvas.setTextColor(TFT_MAGENTA);
  const auto height = canvas.fontHeight();
  const auto width = canvas.fontWidth();
  const auto graphPosOffset = width * 9;
  drawBarGraph(0, 0, graphPosOffset, level, 100, "BAT");
  drawBarGraph(0, height, graphPosOffset, vibration, 20, "V1");
  StickCP2.Display.startWrite();
  canvas.pushSprite(0, 0);
  StickCP2.Display.endWrite();
}

void showProgressMessage(const char* message) {
  static auto cursor = 0;
  const auto width = StickCP2.Display.width();
  const auto height = StickCP2.Display.fontHeight();
  const auto cursorY = (cursor * height) % StickCP2.Display.height();

  Serial.println(message);
  StickCP2.Display.setCursor(0, cursorY);
  StickCP2.Display.fillRect(0, cursorY, width, height, TFT_BLACK);
  StickCP2.Display.printf("%d:", cursor);
  StickCP2.Display.println(message);
  cursor++;
}

int brithness = 10;

void deviceOff() {
  vibrate(0);
  ledcWrite(1, COUNT_HIGH);
}

auto isBlinkLed = false;
auto ledState = 0;

void setup() {
  Serial.begin(115200);

  auto cfg = M5.config();
  StickCP2.begin(cfg);

  StickCP2.Display.begin();
  StickCP2.Display.fillScreen(TFT_BLACK);
  StickCP2.Display.setRotation(1);
  canvas.setTextFont(4);
  canvas.createSprite(
    StickCP2.Display.width(),
    StickCP2.Display.height());

  StickCP2.Display.setTextFont(2);
  StickCP2.Display.setTextColor(TFT_WHITE);
  StickCP2.Display.setCursor(0, 0);
  StickCP2.Display.println("STP: Begin");


  Wire.begin();
  if (!hapDrive.begin())
    showProgressMessage("QHD: Can't comm.");
  else
    showProgressMessage("QHD: DA7280");

  if (!hapDrive.defaultMotor())
    showProgressMessage("QHD: Can't set cfg.");

  // https://learn.sparkfun.com/tutorials/qwiic-haptic-driver-da7280-hookup-guide/example-1-i2c-mode
  hapDrive.enableFreqTrack(false);

  showProgressMessage("QHD: Set I2C Op.");
  hapDrive.setOperationMode(DRO_MODE);
  hapDrive.setVibrate(0);
  showProgressMessage("QHD: Done");

  // For Servo Motor
  showProgressMessage("SRM: Set Srv. Mtr.");
  ledcSetup(1, 50, TIMER_WIDTH);
  ledcAttachPin(26, 1);
  ledcWrite(1, COUNT_HIGH);
  showProgressMessage("SRM: Done");

  showProgressMessage("BLE: Init with \"LVS-\"");
  BLEDevice::init("LVS-");  // CONFIGURATION: The name doesn't actually matter, The app identifies it by the reported id.

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_RX_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  showProgressMessage("BLE: Start service");
  pService->start();

  showProgressMessage("BLE: Start adv.");
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  isBlinkLed = true;

  showProgressMessage("STP: Done");
  StickCP2.Display.setBrightness(brithness);
}

void loop() {
  if (!deviceConnected && oldDeviceConnected) {
    showProgressMessage("BLE: Disc.");
    deviceOff();
    isBlinkLed = true;
    delay(100);
    showProgressMessage("BLE: Start adv.");

    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected) {
    if (!oldDeviceConnected) {
      showProgressMessage("BLE: Conn.");
      oldDeviceConnected = deviceConnected;
    }
    isBlinkLed = false;
    showValues();
  }

  StickCP2.update();
  if (StickCP2.BtnA.wasPressed()) {
    static auto displaySwitch = true;
    displaySwitch = !displaySwitch;
    if (displaySwitch) {
      StickCP2.Display.wakeup();
      StickCP2.Display.setBrightness(brithness);
    } else {
      StickCP2.Display.setBrightness(0);
      StickCP2.Display.sleep();
    }
  }

  if (isBlinkLed) {
    const int sec = millis() / 1000;
    const auto _ledState = sec % 2;
    if (_ledState != ledState) {
      StickCP2.Power.setLed(ledState);
      ledState = _ledState;
    }
  } else {
    if (ledState) {
      ledState = 0;
      StickCP2.Power.setLed(ledState);
    }
  }
}
