#include "HardwareRevX.hpp"
#include "display.hpp"
#include "wifihandler.hpp"

void HardwareRevX::initIO() {
  // Button Pin Definition
  pinMode(SW_1, OUTPUT);
  pinMode(SW_2, OUTPUT);
  pinMode(SW_3, OUTPUT);
  pinMode(SW_4, OUTPUT);
  pinMode(SW_5, OUTPUT);
  pinMode(SW_A, INPUT);
  pinMode(SW_B, INPUT);
  pinMode(SW_C, INPUT);
  pinMode(SW_D, INPUT);
  pinMode(SW_E, INPUT);

  // Power Pin Definition
  pinMode(CRG_STAT, INPUT_PULLUP);
  pinMode(ADC_BAT, INPUT);

  // IR Pin Definition
  pinMode(IR_RX, INPUT);
  pinMode(IR_LED, OUTPUT);
  pinMode(IR_VCC, OUTPUT);
  digitalWrite(IR_LED, HIGH); // HIGH off - LOW on
  digitalWrite(IR_VCC, LOW);  // HIGH on - LOW off

  // LCD Pin Definition
  pinMode(LCD_EN, OUTPUT);
  digitalWrite(LCD_EN, HIGH);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  // Other Pin Definition
  pinMode(ACC_INT, INPUT);
  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, LOW);

  // Release GPIO hold in case we are coming out of standby
  gpio_hold_dis((gpio_num_t)SW_1);
  gpio_hold_dis((gpio_num_t)SW_2);
  gpio_hold_dis((gpio_num_t)SW_3);
  gpio_hold_dis((gpio_num_t)SW_4);
  gpio_hold_dis((gpio_num_t)SW_5);
  gpio_hold_dis((gpio_num_t)LCD_EN);
  gpio_hold_dis((gpio_num_t)LCD_BL);
  gpio_deep_sleep_hold_dis();
}

HardwareRevX::HardwareRevX() : HardwareAbstract() {}

HardwareRevX::WakeReason getWakeReason() {
  // Find out wakeup cause
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    if (log(esp_sleep_get_ext1_wakeup_status()) / log(2) == 13)
      return HardwareRevX::WakeReason::IMU;
    else
      return HardwareRevX::WakeReason::KEYPAD;
  } else {
    return HardwareRevX::WakeReason::RESET;
  }
}

void HardwareRevX::init() {
  // Make sure ESP32 is running at full speed
  setCpuFrequencyMhz(240);
  wakeup_reason = getWakeReason();
  initIO();
  Serial.begin(115200);

  mDisplay = Display::getInstance();
  mBattery = std::make_shared<Battery>(ADC_BAT, CRG_STAT);
  mWifiHandler = wifiHandler::getInstance();
  mKeys = std::make_shared<Keys>();
  restorePreferences();

  mTouchHandler.SetNotification(mDisplay->TouchNotification());
  mTouchHandler = [this]([[maybe_unused]] auto touchPoint) {
    standbyTimer = this->getSleepTimeout();
  };

  setupIMU();
  setupIR();

  debugPrint("Finished Hardware Setup in %d", millis());
}

void HardwareRevX::debugPrint(const char *fmt, ...) {
  char result[100];
  va_list arguments;

  va_start(arguments, fmt);
  vsnprintf(result, 100, fmt, arguments);
  va_end(arguments);

  Serial.print(result);
}

std::shared_ptr<wifiHandlerInterface> HardwareRevX::wifi() {
  return mWifiHandler;
}

std::shared_ptr<BatteryInterface> HardwareRevX::battery() { return mBattery; }

std::shared_ptr<DisplayAbstract> HardwareRevX::display() { return mDisplay; }

std::shared_ptr<KeyPressAbstract> HardwareRevX::keys() { return mKeys; }

void HardwareRevX::activityDetection() {
  static int accXold;
  static int accYold;
  static int accZold;
  int accX = IMU.readFloatAccelX() * 1000;
  int accY = IMU.readFloatAccelY() * 1000;
  int accZ = IMU.readFloatAccelZ() * 1000;

  // determine motion value as da/dt
  motion = (abs(accXold - accX) + abs(accYold - accY) + abs(accZold - accZ));
  // Calculate time to standby
  standbyTimer -= 100;
  if (standbyTimer < 0)
    standbyTimer = 0;
  // If the motion exceeds the threshold, the standbyTimer is reset
  if (motion > MOTION_THRESHOLD)
    standbyTimer = sleepTimeout;

  // Store the current acceleration and time
  accXold = accX;
  accYold = accY;
  accZold = accZ;
}

char HardwareRevX::getCurrentDevice() { return currentDevice; }

void HardwareRevX::setCurrentDevice(char currentDevice) {
  this->currentDevice = currentDevice;
}

bool HardwareRevX::getWakeupByIMUEnabled() { return wakeupByIMUEnabled; }

void HardwareRevX::setWakeupByIMUEnabled(bool wakeupByIMUEnabled) {
  this->wakeupByIMUEnabled = wakeupByIMUEnabled;
}

uint16_t HardwareRevX::getSleepTimeout() { return sleepTimeout; }

void HardwareRevX::setSleepTimeout(uint16_t sleepTimeout) {
  this->sleepTimeout = sleepTimeout;
  standbyTimer = sleepTimeout;
}

void HardwareRevX::enterSleep() {
  // Save settings to internal flash memory
  preferences.putBool("wkpByIMU", wakeupByIMUEnabled);
  preferences.putUChar("blBrightness", mDisplay->getBrightness());
  preferences.putUChar("currentDevice", currentDevice);
  preferences.putUInt("sleepTimeout", sleepTimeout);
  if (!preferences.getBool("alreadySetUp"))
    preferences.putBool("alreadySetUp", true);
  preferences.end();

  // Configure IMU
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC); // clear interrupt
  configIMUInterrupts();
  IMU.readRegister(&intDataRead,
                   LIS3DH_INT1_SRC); // really clear interrupt

  // Prepare IO states
  digitalWrite(LCD_DC, LOW); // LCD control signals off
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_MOSI, LOW);
  digitalWrite(LCD_SCK, LOW);
  digitalWrite(LCD_EN, HIGH); // LCD logic off
  digitalWrite(LCD_BL, HIGH); // LCD backlight off
  pinMode(CRG_STAT, INPUT);   // Disable Pull-Up
  digitalWrite(IR_VCC, LOW);  // IR Receiver off

  // Configure button matrix for ext1 interrupt
  pinMode(SW_1, OUTPUT);
  pinMode(SW_2, OUTPUT);
  pinMode(SW_3, OUTPUT);
  pinMode(SW_4, OUTPUT);
  pinMode(SW_5, OUTPUT);
  digitalWrite(SW_1, HIGH);
  digitalWrite(SW_2, HIGH);
  digitalWrite(SW_3, HIGH);
  digitalWrite(SW_4, HIGH);
  digitalWrite(SW_5, HIGH);
  gpio_hold_en((gpio_num_t)SW_1);
  gpio_hold_en((gpio_num_t)SW_2);
  gpio_hold_en((gpio_num_t)SW_3);
  gpio_hold_en((gpio_num_t)SW_4);
  gpio_hold_en((gpio_num_t)SW_5);
  // Force display pins to high impedance
  // Without this the display might not wake up from sleep
  pinMode(LCD_BL, INPUT);
  pinMode(LCD_EN, INPUT);
  gpio_hold_en((gpio_num_t)LCD_BL);
  gpio_hold_en((gpio_num_t)LCD_EN);
  gpio_deep_sleep_hold_en();

  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  delay(100);
  // Sleep
  esp_deep_sleep_start();
}

void HardwareRevX::configIMUInterrupts() {
  uint8_t dataToWrite = 0;

  // LIS3DH_INT1_CFG
  // dataToWrite |= 0x80;//AOI, 0 = OR 1 = AND
  // dataToWrite |= 0x40;//6D, 0 = interrupt source, 1 = 6 direction source
  // Set these to enable individual axes of generation source (or direction)
  //  -- high and low are used generically
  dataToWrite |= 0x20; // Z high
  // dataToWrite |= 0x10;//Z low
  dataToWrite |= 0x08; // Y high
  // dataToWrite |= 0x04;//Y low
  dataToWrite |= 0x02; // X high
  // dataToWrite |= 0x01;//X low
  if (wakeupByIMUEnabled)
    IMU.writeRegister(LIS3DH_INT1_CFG, 0b00101010);
  else
    IMU.writeRegister(LIS3DH_INT1_CFG, 0b00000000);

  // LIS3DH_INT1_THS
  dataToWrite = 0;
  // Provide 7 bit value, 0x7F always equals max range by accelRange setting
  dataToWrite |= 0x45;
  IMU.writeRegister(LIS3DH_INT1_THS, dataToWrite);

  // LIS3DH_INT1_DURATION
  dataToWrite = 0;
  // minimum duration of the interrupt
  // LSB equals 1/(sample rate)
  dataToWrite |= 0x00; // 1 * 1/50 s = 20ms
  IMU.writeRegister(LIS3DH_INT1_DURATION, dataToWrite);

  // LIS3DH_CTRL_REG5
  // Int1 latch interrupt and 4D on  int1 (preserve fifo en)
  IMU.readRegister(&dataToWrite, LIS3DH_CTRL_REG5);
  dataToWrite &= 0xF3; // Clear bits of interest
  dataToWrite |= 0x08; // Latch interrupt (Cleared by reading int1_src)
  // dataToWrite |= 0x04; //Pipe 4D detection from 6D recognition to int1?
  IMU.writeRegister(LIS3DH_CTRL_REG5, dataToWrite);

  // LIS3DH_CTRL_REG3
  // Choose source for pin 1
  dataToWrite = 0;
  // dataToWrite |= 0x80; //Click detect on pin 1
  dataToWrite |= 0x40; // AOI1 event (Generator 1 interrupt on pin 1)
  dataToWrite |= 0x20; // AOI2 event ()
  // dataToWrite |= 0x10; //Data ready
  // dataToWrite |= 0x04; //FIFO watermark
  // dataToWrite |= 0x02; //FIFO overrun
  IMU.writeRegister(LIS3DH_CTRL_REG3, dataToWrite);
}

void HardwareRevX::restorePreferences() {
  // Restore settings from internal flash memory
  int backlight_brightness = 255;
  preferences.begin("settings", false);
  if (preferences.getBool("alreadySetUp")) {
    wakeupByIMUEnabled = preferences.getBool("wkpByIMU");
    backlight_brightness = preferences.getUChar("blBrightness");
    currentDevice = preferences.getUChar("currentDevice");
    sleepTimeout = preferences.getUInt("sleepTimeout");
    // setting the default to prevent a 0ms sleep timeout
    if (sleepTimeout == 0) {
      sleepTimeout = SLEEP_TIMEOUT;
    }
  }
  mDisplay->setBrightness(backlight_brightness);
}

void HardwareRevX::setupIMU() {
  // Setup hal
  IMU.settings.accelSampleRate =
      50; // Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  IMU.settings.accelRange = 2; // Max G force readable.  Can be: 2, 4, 8, 16
  IMU.settings.adcEnabled = 0;
  IMU.settings.tempEnabled = 0;
  IMU.settings.xAccelEnabled = 1;
  IMU.settings.yAccelEnabled = 1;
  IMU.settings.zAccelEnabled = 1;
  IMU.begin();
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC); // clear interrupt
}

void HardwareRevX::setupIR() {
  // Setup IR
  IrSender.begin();
  digitalWrite(IR_VCC, HIGH); // Turn on IR receiver
  IrReceiver.enableIRIn();    // Start the receiver
}

void HardwareRevX::startTasks() {}

void HardwareRevX::loopHandler() {
  standbyTimer < 2000 ? mDisplay->sleep() : mDisplay->wake();

  // TODO move to debug task
  // Blink debug LED at 1 Hz
  digitalWrite(USER_LED, millis() % 1000 > 500);

  // Refresh IMU data at 10Hz
  static unsigned long IMUTaskTimer = millis();
  if (millis() - IMUTaskTimer >= 100) {
    activityDetection();
    if (standbyTimer == 0) {
      Serial.println("Entering Sleep Mode. Goodbye.");
      enterSleep();
    }
    IMUTaskTimer = millis();
  }
}