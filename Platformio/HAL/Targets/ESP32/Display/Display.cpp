#include <Display.hpp>
#include <Wire.h>
#include <Preferences.h>
//#include "lv_fs_littlefs.h"

// LVGL declarations
LV_IMG_DECLARE(gradientLeft);
LV_IMG_DECLARE(gradientRight);
LV_IMG_DECLARE(lightbulb);

static lv_disp_drv_t disp_drv;

std::shared_ptr<Display> Display::getInstance() {
  if (DisplayAbstract::mInstance == nullptr) {
    DisplayAbstract::mInstance =
        std::shared_ptr<Display>(new Display(LCD_BL, LCD_EN));
  }
  return std::static_pointer_cast<Display>(mInstance);
}

Display::Display(int backlight_pin, int enable_pin) : DisplayAbstract() //, backlight_pin(backlight_pin), enable_pin(enable_pin), tft(TFT_eSPI()), touch(Adafruit_FT6206())
{
  backlight_pin = backlight_pin;
  enable_pin = enable_pin;
  // LCD Pin Definition
  pinMode(this->enable_pin, OUTPUT);
  digitalWrite(this->enable_pin, HIGH);
  pinMode(this->backlight_pin, OUTPUT);
  digitalWrite(this->backlight_pin, HIGH);

  this->tft = TFT_eSPI();
#if 1
  ledcSetup(LCD_BACKLIGHT_LEDC_CHANNEL, LCD_BACKLIGHT_LEDC_FREQUENCY, LCD_BACKLIGHT_LEDC_BIT_RESOLUTION);
  ledcAttachPin(this->backlight_pin, LCD_BACKLIGHT_LEDC_CHANNEL);
  ledcWrite(LCD_BACKLIGHT_LEDC_CHANNEL, 0);
#else
  // Configure the backlight PWM
  // Manual setup because ledcSetup() briefly turns on the backlight
  ledc_channel_config_t ledc_channel_left;
  ledc_channel_left.gpio_num = (gpio_num_t)this->backlight_pin;
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
#endif

  // Slowly charge the VSW voltage to prevent a brownout
  // Workaround for hardware rev 1!
  for (int i = 0; i < 100; i++)
  {
    digitalWrite(this->enable_pin, HIGH); // LCD Logic off
    delayMicroseconds(1);
    digitalWrite(this->enable_pin, LOW); // LCD Logic on
  }

  delay(100); // Wait for the LCD driver to power on
  this->tft.init();
  this->tft.initDMA();
  this->tft.setRotation(0);
  this->tft.fillScreen(TFT_BLACK);
  this->tft.setSwapBytes(true);

  // TODO: move touchscreen handling out of Display class
  //  Setup touchscreen
  // this->touch = Adafruit_FT6206();
  Wire.begin(19, 22, 400000); // Configure i2c pins and set frequency to 400kHz
  // this->touch.begin(128); // Initialize touchscreen and set sensitivity threshold

  // this->backlight_brightness = preferences.getUChar("blBrightness", DEFAULT_BACKLIGHT_BRIGHTNESS);
  // LV_LOG_TRACE("restore blBrightness to %d", this->backlight_brightness);
  Preferences preferences;
  mBrightness = preferences.getUChar("blBrightness", DEFAULT_BACKLIGHT_BRIGHTNESS);
  LV_LOG_TRACE("restore blBrightness to %d", mBrightness);

  // Setup LVGL
  lv_init();
  //lv_port_littlefs_init();
}

void Display::fadeImpl(void *) {
  bool fadeDone = false;
  while (!fadeDone) {
    fadeDone = getInstance()->fade();
    vTaskDelay(3 / portTICK_PERIOD_MS); // 3 miliseconds between steps
    // 0 - 255 will take about .75 seconds to fade up.
  }

  xSemaphoreTake(getInstance()->mFadeTaskMutex, portMAX_DELAY);
  getInstance()->mDisplayFadeTask = nullptr;
  xSemaphoreGive(getInstance()->mFadeTaskMutex);

  vTaskDelete(nullptr); // Delete Fade Task
}

void Display::setBrightness(uint8_t brightness) {
  mAwakeBrightness = brightness;
  Serial.print("Set Brightness:");
  Serial.println(mAwakeBrightness);
  startFade();
}

uint8_t Display::getBrightness() { return mAwakeBrightness; }

void Display::setCurrentBrightness(uint8_t brightness) {
  mBrightness = brightness;
  auto duty = static_cast<int>(mBrightness);
  ledcWrite(LCD_BACKLIGHT_LEDC_CHANNEL, duty);
  // Serial.print("Current Brightness:");
  // Serial.println(mBrightness);
}

bool Display::fade() {
  // Early return no fade needed.
  if (mBrightness == mAwakeBrightness || isAsleep && mBrightness == 0) {
    return true;
  }

  bool fadeDown = isAsleep || mBrightness > mAwakeBrightness;
  if (fadeDown) {
    setCurrentBrightness(mBrightness - 1);
    auto setPoint = isAsleep ? 0 : mAwakeBrightness;
    return mBrightness == setPoint;
  } else {
    setCurrentBrightness(mBrightness + 1);
    return mBrightness == mAwakeBrightness;
  }
}

void Display::startFade() {
  xSemaphoreTake(mFadeTaskMutex, portMAX_DELAY);
  // Only Create Task if it is needed
  if (mDisplayFadeTask == nullptr) {
    xTaskCreate(&Display::fadeImpl, "Display Fade Task", 1024, nullptr, 5,
                &mDisplayFadeTask);
  }
  xSemaphoreGive(mFadeTaskMutex);
}

void Display::turnOff()
{
  digitalWrite(this->backlight_pin, HIGH);
  digitalWrite(this->enable_pin, HIGH);
  pinMode(this->backlight_pin, INPUT);
  pinMode(this->enable_pin, INPUT);
  gpio_hold_en((gpio_num_t)this->backlight_pin);
  gpio_hold_en((gpio_num_t)this->enable_pin);

  // LV_LOG_TRACE("save blBrightness %d", this->backlight_brightness);
  // preferences.putUChar("blBrightness", this->backlight_brightness);
  // /* save settings of all devices */
  // settings.saveSettings();
  LV_LOG_TRACE("save blBrightness %d", mBrightness);
  Preferences preferences;
  preferences.begin("DisplaySettings", false);
  preferences.putUChar("blBrightness", mBrightness);
}

static lv_disp_draw_buf_t draw_buf;

/* Callback function to show and hide keybaord for attached textareas */
static void ta_kb_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  switch (code)
  {
  case LV_EVENT_FOCUSED:
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(kb);
    break;
  case LV_EVENT_DEFOCUSED:
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    break;
  default:
    break;
  }
}

//void Display::flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
void Display::flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  this->tft.startWrite();
  this->tft.setAddrWindow(area->x1, area->y1, w, h);
  this->tft.pushPixelsDMA((uint16_t *)&color_p->full, w * h);
  this->tft.endWrite();

  lv_disp_flush_ready(disp);
}

void Display::update()
{
  static int fadeInTimer = millis(); // fadeInTimer = time after setup
  if (millis() < fadeInTimer + backlight_brightness)
  { // Fade in the backlight brightness
    ledcWrite(5, millis() - fadeInTimer);
  }
  else
  { // Dim Backlight before entering standby
    // if(standbyTimer < 2000) ledcWrite(5, 85); // Backlight dim
    // else ledcWrite(5, this->backlight_brightness);  // Backlight on
    ledcWrite(5, backlight_brightness); // Backlight on
  }
  // Update LVGL UI
  lv_timer_handler();
}
