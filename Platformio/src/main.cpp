// OMOTE firmware for ESP32
// 2023 Maximilian Kern

#include <TFT_eSPI.h> // Hardware-specific library
#include <Keypad.h>   // modified for inverted logic
#include <Preferences.h>
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include <lvgl.h>
#include "WiFi.h"
#include <Adafruit_FT6206.h>
#include "driver/ledc.h"
#include <PubSubClient.h>

#include <omote.hpp>
#include <Display.hpp>
#include <WifiHandler.hpp>
#include <Battery.hpp>
#include <IRHandler.hpp>
#include <Settings.hpp>
#include <MagentaTV.hpp>
#include <AppleTV.hpp>
#include <SmartHome.hpp>

#define ENABLE_WIFI // Comment out to diable connected features

// Variables and Object declarations ------------------------------------------------------------------------------------------------------
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
extern IRrecv IrReceiver;
extern decode_results results;
const uint8_t kTolerancePercentage = kTolerance;
const uint16_t kCaptureBufferSize = 1024;

// Battery declares
int battery_voltage = 0;
int battery_percentage = 100;
bool battery_ischarging = false;

// IMU declarations
int motion = 0;
#define SLEEP_TIMEOUT 10000 // time until device enters sleep mode in milliseconds
#define MOTION_THRESHOLD 50 // motion above threshold keeps device awake
long standbyTimerConfigured = SLEEP_TIMEOUT;
long standbyTimer = standbyTimerConfigured;
bool wakeupByIMUEnabled = true;
LIS3DH IMU(I2C_MODE, 0x19); // Default constructor is I2C, addr 0x19.

// LCD declarations
TFT_eSPI tft = TFT_eSPI();
#define screenWidth 240
#define screenHeight 320
Adafruit_FT6206 touch = Adafruit_FT6206();
TS_Point touchPoint;
TS_Point oldPoint;

/* HAL instances */
Display display(LCD_BL, LCD_EN, screenWidth, screenHeight);
WifiHandler wifihandler;
Battery battery(ADC_BAT, CRG_STAT);
IRHandler irhandler;

/* UI instances */
Settings settings(&display);

// App instances
MagentaTV magentaTV(&display);
AppleTV appletv(&display);
SmartHome smarthome(&display);

// Keypad declarations
const byte ROWS = 5; // four rows
const byte COLS = 5; // four columns
// define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
    {'s', '^', '-', 'm', 'r'}, //  source, channel+, Volume-,   mute, record
    {'i', 'r', '+', 'k', 'd'}, //    info,    right, Volume+,     OK,   down
    {'4', 'v', '1', '3', '2'}, //    blue, channel-,     red, yellow,  green
    {'>', 'o', 'b', 'u', 'l'}, // forward,      off,    back,     up,   left
    {'?', 'p', 'c', '<', '='}  //       ?,     play,  config, rewind,   stop
};
byte rowPins[ROWS] = {SW_A, SW_B, SW_C, SW_D, SW_E}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {SW_1, SW_2, SW_3, SW_4, SW_5}; // connect to the column pinouts of the keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
#define BUTTON_PIN_BITMASK 0b1110110000000000000000000010000000000000 // IO34+IO35+IO37+IO38+IO39(+IO13)
// byte keyMapMagentaTV[ROWS][COLS] = {
//     {0x69, 0x20, 0x11, 0x0D, 0x56},
//     {0x4F, 0x37, 0x10, 0x57, 0x51},
//     {0x6E, 0x21, 0x6B, 0x6D, 0x6C},
//     {0x34, 0x0C, 0x22, 0x50, 0x55},
//     {'?', 0x35, 0x2F, 0x32, 0x36}};
byte currentDevice = 1; // Current Device to control (allows switching mappings between devices)

// Other declarations
byte wakeup_reason;
enum Wakeup_reasons
{
  WAKEUP_BY_RESET,
  WAKEUP_BY_IMU,
  WAKEUP_BY_KEYPAD
};
Preferences preferences;

// #define WIFI_SSID "YOUR_WIFI_SSID"
// #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
// #define MQTT_SERVER "YOUR_MQTT_SERVER_IP"
extern WiFiClient espClient;
PubSubClient client(espClient);

// LVGL declarations
lv_obj_t *panel;

// Helper Functions -----------------------------------------------------------------------------------------------------------------------


// Display flushing
/*
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ){
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushPixelsDMA( ( uint16_t * )&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready( disp );
}
*/

// Read the touchpad
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  // int16_t touchX, touchY;
  touchPoint = touch.getPoint();
  int16_t touchX = touchPoint.x;
  int16_t touchY = touchPoint.y;
  bool touched = false;
  if ((touchX > 0) || (touchY > 0))
  {
    touched = true;
    standbyTimer = standbyTimerConfigured;
  }

  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    // Set the coordinates
    data->point.x = screenWidth - touchX;
    data->point.y = screenHeight - touchY;

    // tft.drawFastHLine(0, screenHeight - touchY, screenWidth, TFT_RED);
    // tft.drawFastVLine(screenWidth - touchX, 0, screenHeight, TFT_RED);
  }
}

void activityDetection()
{
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
    standbyTimer = standbyTimerConfigured;

  // Store the current acceleration and time
  accXold = accX;
  accYold = accY;
  accZold = accZ;
}

void configIMUInterrupts()
{
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

void enterSleep()
{
  Serial.println("enterSleep called");

  // Configure IMU
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC); // clear interrupt
  configIMUInterrupts();
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC); // really clear interrupt

#ifdef ENABLE_WIFI
  wifihandler.turnOff();
// Power down modem
#endif

  display.turnOff();

  // Save settings to internal flash memory
  preferences.putBool("wkpByIMU", wakeupByIMUEnabled);
  preferences.putUChar("currentDevice", currentDevice);
  preferences.putLong("standbyTimer", standbyTimerConfigured);
  Serial.printf("enterSleep: standbyTimer: %d standbyTimerConfigured: %d", standbyTimer, standbyTimerConfigured);
  if (!preferences.getBool("alreadySetUp"))
    preferences.putBool("alreadySetUp", true);
  preferences.end();
  // Prepare IO states
  digitalWrite(LCD_DC, LOW); // LCD control signals off
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_MOSI, LOW);
  digitalWrite(LCD_SCK, LOW);
  pinMode(CRG_STAT, INPUT);  // Disable Pull-Up
  digitalWrite(IR_VCC, LOW); // IR Receiver off

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
  gpio_deep_sleep_hold_en();

  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  delay(100);
  // Sleep
  esp_deep_sleep_start();
}

// Setup ----------------------------------------------------------------------------------------------------------------------------------

void setup()
{

  setCpuFrequencyMhz(240); // Make sure ESP32 is running at full speed

  // Find out wakeup cause
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1)
  {
    if (log(esp_sleep_get_ext1_wakeup_status()) / log(2) == 13)
      wakeup_reason = WAKEUP_BY_IMU;
    else
      wakeup_reason = WAKEUP_BY_KEYPAD;
  }
  else
  {
    wakeup_reason = WAKEUP_BY_RESET;
  }

  // --- IO Initialization ---

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

  // Configure the backlight PWM
  // Manual setup because ledcSetup() briefly turns on the backlight
  ledc_channel_config_t ledc_channel_left;
  ledc_channel_left.gpio_num = (gpio_num_t)LCD_BL;
  ledc_channel_left.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel_left.channel = LEDC_CHANNEL_5;
  ledc_channel_left.intr_type = LEDC_INTR_DISABLE;
  ledc_channel_left.timer_sel = LEDC_TIMER_1;
  ledc_channel_left.flags.output_invert = 1; // Can't do this with ledcSetup()
  ledc_channel_left.duty = 0;

  ledc_timer_config_t ledc_timer;
  ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_timer.duty_resolution = LEDC_TIMER_8_BIT;
  ledc_timer.timer_num = LEDC_TIMER_1;
  ledc_timer.freq_hz = 640;

  ledc_channel_config(&ledc_channel_left);
  ledc_timer_config(&ledc_timer);

  // --- Startup ---

  Serial.begin(115200);

  Serial.println("starting setup");

  // Restore settings from internal flash memory
  preferences.begin("settings", false);
  if (preferences.getBool("alreadySetUp"))
  {
    wakeupByIMUEnabled = preferences.getBool("wkpByIMU");
    // backlight_brightness = preferences.getUChar("blBrightness");
    currentDevice = preferences.getUChar("currentDevice");
    standbyTimerConfigured = preferences.getLong("standbyTimer", 10000);
    standbyTimer = standbyTimerConfigured;
    Serial.printf("restore: standbyTimer: %d standbyTimerConfigured: %d", standbyTimer, standbyTimerConfigured);
  }

  // Setup IR
  Serial.println("init IR");
  // IrSender.begin();
  // digitalWrite(IR_VCC, HIGH); // Turn on IR receiver
  // IrReceiver.enableIRIn();    // Start the receiver
  irhandler.setup();

  // Setup TFT
  Serial.println("init TFT");
  display.setup();
  // --- LVGL UI Configuration ---
  display.setup_ui();

  // setup app UIs
  magentaTV.setup();
  appletv.setup();
  smarthome.setup();

  // Settings Menu in last spot
  settings.setup();

  display.setActiveTab(currentDevice);

  // --- End of LVGL configuration ---
  touch.begin();

#ifdef ENABLE_WIFI
  // Setup WiFi
  Serial.println("init WIFI");
  wifihandler.begin();
#endif

  // Setup IMU
  Serial.println("init IMU");
  IMU.settings.accelSampleRate = 50; // Hz.  Can be: 0,1,10,25,50,100,200,400,1600,5000 Hz
  IMU.settings.accelRange = 2;       // Max G force readable.  Can be: 2, 4, 8, 16
  IMU.settings.adcEnabled = 0;
  IMU.settings.tempEnabled = 0;
  IMU.settings.xAccelEnabled = 1;
  IMU.settings.yAccelEnabled = 1;
  IMU.settings.zAccelEnabled = 1;
  IMU.begin();
  uint8_t intDataRead;
  IMU.readRegister(&intDataRead, LIS3DH_INT1_SRC); // clear interrupt

  Serial.println("lv_timer_handler()");
  lv_timer_handler(); // Run the LVGL UI once before the loop takes over
  Serial.println("after first lv_timer_handler() call");

  Serial.print("Setup finised in ");
  Serial.print(millis());
  Serial.println("ms.");
}

// Loop ------------------------------------------------------------------------------------------------------------------------------------
void loop()
{

  display.update();
  // Blink debug LED at 1 Hz
  digitalWrite(USER_LED, millis() % 1000 > 500);

  // Refresh IMU data at 10Hz
  static unsigned long IMUTaskTimer = millis();
  if (millis() - IMUTaskTimer >= 100)
  {
    activityDetection();
    if (standbyTimer == 0)
    {
      Serial.println("Entering Sleep Mode. Goodbye.");
      enterSleep();
    }
    IMUTaskTimer = millis();
  }

#if 1
  // Update battery stats at 1Hz
  static unsigned long batteryTaskTimer = millis() + 1000; // add 1s to start immediately
  if (millis() - batteryTaskTimer >= 1000)
  {
#if 0
    battery_voltage = analogRead(ADC_BAT)*2*3300/4095 + 350; // 350mV ADC offset
    battery_percentage = constrain(map(battery_voltage, 3700, 4200, 0, 100), 0, 100);
    batteryTaskTimer = millis();
    battery_ischarging = !digitalRead(CRG_STAT);
    // Check if battery is charging, fully charged or disconnected
    if(battery_ischarging || (!battery_ischarging && battery_voltage > 4350)){
      lv_label_set_text(objBattPercentage, "");
      lv_label_set_text(objBattIcon, LV_SYMBOL_USB);
    }
    else{
      // Update status bar battery indicator
      //lv_label_set_text_fmt(objBattPercentage, "%d%%", battery_percentage);
      if(battery_percentage > 95) lv_label_set_text(objBattIcon, LV_SYMBOL_BATTERY_FULL);
      else if(battery_percentage > 75) lv_label_set_text(objBattIcon, LV_SYMBOL_BATTERY_3);
      else if(battery_percentage > 50) lv_label_set_text(objBattIcon, LV_SYMBOL_BATTERY_2);
      else if(battery_percentage > 25) lv_label_set_text(objBattIcon, LV_SYMBOL_BATTERY_1);
      else lv_label_set_text(objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
    }
#else
    batteryTaskTimer = millis();
    battery.update();
#endif
  }
#endif

  // Keypad Handling
  customKeypad.getKey(); // Populate key list
  for (int i = 0; i < LIST_MAX; i++)
  { // Handle multiple keys (Not really necessary in this case)
    if (customKeypad.key[i].kstate == PRESSED) // || customKeypad.key[i].kstate == HOLD)
    {
      standbyTimer = standbyTimerConfigured; // Reset the sleep timer when a button is pressed
      //int keyCode = customKeypad.key[i].kcode;
      Serial.printf("pressed: %c\n", customKeypad.key[i].kchar);
      // Send IR codes depending on the current device (tabview page)
      display.getApp(currentDevice)->handleCustomKeypad(customKeypad.key[i].kcode, customKeypad.key[i].kchar);
    } else if (customKeypad.key[i].kstate == RELEASED) {
      Serial.printf("released: %c\n", customKeypad.key[i].kchar);
    }
  }

  if (IrReceiver.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }

  // IR Test
  // tft.drawString("IR Command: ", 10, 90, 1);
  // decode_results results;
  // if (IrReceiver.decode(&results)) {
  //  //tft.drawString(String(results.command) + "        ", 80, 90, 1);
  //  IrReceiver.resume(); // Enable receiving of the next value
  //}
}