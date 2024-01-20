#include <Arduino.h>
#include <Display.hpp>
#include <Wire.h>
#include <Preferences.h>
#include <Settings.hpp>

extern bool wakeupByIMUEnabled;
extern Display display;
extern byte currentDevice;
extern Settings settings;

// LVGL declarations
LV_IMG_DECLARE(gradientLeft);
LV_IMG_DECLARE(gradientRight);
LV_IMG_DECLARE(lightbulb);

extern lv_obj_t *panel;
static lv_disp_drv_t disp_drv;

/*TODO: get rid of global variable and use API functions instead*/
extern Preferences preferences;

// TODO: fix callback function structure to pass it in and/or move it out of display class somehow else
void smartHomeSlider_event_cb(lv_event_t *e);
void smartHomeToggle_event_cb(lv_event_t *e);
//void WakeEnableSetting_event_cb(lv_event_t *e);
//void appleKey_event_cb(lv_event_t *e);
//void virtualKeypad_event_cb(lv_event_t *e);
//void bl_slider_event_cb(lv_event_t *e);
//void tabview_device_event_cb(lv_event_t *e);
//void store_scroll_value_event_cb(lv_event_t *e);
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

// Set the page indicator scroll position relative to the tabview scroll position
void store_scroll_value_event_cb(lv_event_t* e){
  float bias = (150.0 + 8.0) / 240.0;
  int offset = 240 / 2 - 150 / 2 - 8 - 50 - 3;
  lv_obj_t* screen = lv_event_get_target(e);
  lv_obj_scroll_to_x(panel, lv_obj_get_scroll_x(screen) * bias - offset, LV_ANIM_OFF);
}

// Update current device when the tabview page is changes
void tabview_device_event_cb(lv_event_t* e){
  currentDevice = lv_tabview_get_tab_act(lv_event_get_target(e));
  Serial.printf("slide to currentDevice %d\n", currentDevice);
}

// Display flushing
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  display.flush(disp, area, color_p);
}

Display::Display(int backlight_pin, int enable_pin, int width, int height)
{
  this->backlight_pin = backlight_pin;
  this->enable_pin = enable_pin;
  this->width = width;
  this->height = height;
  this->primary_color = lv_color_hex(0x303030); // gray

  /*Initialize the keybard as null*/
  this->kb = NULL;

  /*Initialize tab name array*/
  for (int i = 0; i < TAB_ARRAY_SIZE; i++)
  {
    this->apps[i] = nullptr;
  }
}

lv_color_t Display::getPrimaryColor()
{
  return this->primary_color;
}

unsigned int *Display::getBacklightBrightness()
{
  return &this->backlight_brightness;
}

void Display::hide_keyboard()
{
  lv_obj_add_flag(this->kb, LV_OBJ_FLAG_HIDDEN);
}

void Display::show_keyboard()
{
  lv_obj_clear_flag(this->kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(this->kb);
}

void Display::turnOff()
{
  digitalWrite(this->backlight_pin, HIGH);
  digitalWrite(this->enable_pin, HIGH);
  pinMode(this->backlight_pin, INPUT);
  pinMode(this->enable_pin, INPUT);
  gpio_hold_en((gpio_num_t)this->backlight_pin);
  gpio_hold_en((gpio_num_t)this->enable_pin);
  Serial.printf("save blBrightness %d\n", this->backlight_brightness);
  preferences.putUChar("blBrightness", this->backlight_brightness);
  /* save settings of all devices */
  settings.saveDeviceSettings();
}

void Display::setup()
{
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

  this->backlight_brightness = preferences.getUChar("blBrightness", DEFAULT_BACKLIGHT_BRIGHTNESS);
  Serial.printf("restore blBrightness to %d\n", this->backlight_brightness);

  // Setup LVGL
  lv_init();
}

static lv_disp_draw_buf_t draw_buf;
void Display::setup_ui()
{
  Serial.println("Display::setup_ui()");
  this->bufA = (lv_color_t *)malloc((this->width * this->height / 10) * sizeof(lv_color_t));
  this->bufB = (lv_color_t *)malloc((this->width * this->height / 10) * sizeof(lv_color_t));

  lv_disp_draw_buf_init(&draw_buf, this->bufA, this->bufB, this->width * this->height / 10);
  // Initialize the display driver
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = this->width;
  disp_drv.ver_res = this->height;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // TODO: move touchscreen driver to its own module
  //  Initialize the touchscreen driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  // Set the background color
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

  this->create_keyboard();
  // Setup a scrollable tabview for devices and settings
  this->tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 0); // Hide tab labels by setting their height to 0
  lv_obj_set_style_bg_color(this->tabview, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_size(this->tabview, this->width, 270); // 270 = screenHeight(320) - panel(30) - statusbar(20)
  lv_obj_align(this->tabview, LV_ALIGN_TOP_MID, 0, 20);

  // Configure number button grid
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
  static lv_coord_t row_dsc[] = {52, 52, 52, 52, LV_GRID_TEMPLATE_LAST};                              // manual y distribution to compress the grid a bit

  // Set current page according to the current Device
  lv_tabview_set_act(this->tabview, 0, LV_ANIM_OFF);

  // Make the indicator scroll together with the tabs by creating a scroll event
  lv_obj_add_event_cb(lv_tabview_get_content(tabview), store_scroll_value_event_cb, LV_EVENT_SCROLL, NULL);
  lv_obj_add_event_cb(this->tabview, tabview_device_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  // Initialize scroll position for the indicator
  //lv_event_send(lv_tabview_get_content(this->tabview), LV_EVENT_SCROLL, NULL);

  // Create a status bar
  lv_obj_t *statusbar = lv_btn_create(lv_scr_act());
  lv_obj_set_size(statusbar, 240, 20);
  lv_obj_set_style_shadow_width(statusbar, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(statusbar, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_radius(statusbar, 0, LV_PART_MAIN);
  lv_obj_align(statusbar, LV_ALIGN_TOP_MID, 0, 0);

  this->WifiLabel = lv_label_create(statusbar);
  lv_label_set_text(this->WifiLabel, "");
  lv_obj_align(this->WifiLabel, LV_ALIGN_LEFT_MID, -8, 0);
  lv_obj_set_style_text_font(this->WifiLabel, &lv_font_montserrat_12, LV_PART_MAIN);

  this->objBattPercentage = lv_label_create(statusbar);
  lv_label_set_text(this->objBattPercentage, "");
  lv_obj_align(this->objBattPercentage, LV_ALIGN_RIGHT_MID, -16, 0);
  lv_obj_set_style_text_font(this->objBattPercentage, &lv_font_montserrat_12, LV_PART_MAIN);

  this->objBattIcon = lv_label_create(statusbar);
  lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_align(this->objBattIcon, LV_ALIGN_RIGHT_MID, 8, 0);
  lv_obj_set_style_text_font(this->objBattIcon, &lv_font_montserrat_16, LV_PART_MAIN);

  this->objUSBIcon = lv_label_create(statusbar);
  lv_label_set_text(this->objUSBIcon, LV_SYMBOL_USB);
  lv_obj_align(this->objUSBIcon, LV_ALIGN_RIGHT_MID, -40, 0);
  lv_obj_set_style_text_font(this->objUSBIcon, &lv_font_montserrat_16, LV_PART_MAIN);
}

lv_obj_t *Display::getTabView()
{
  return this->tabview;
}

// lv_obj_t *wifi_subpage;
// static lv_obj_t *kb;
// static lv_obj_t *ta;

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

void Display::create_keyboard()
{
  this->kb = lv_keyboard_create(lv_scr_act());
  lv_obj_add_flag(this->kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_y(this->kb, 0);
}

void Display::attach_keyboard(lv_obj_t *textarea)
{
  if (this->kb == NULL)
  {
    this->create_keyboard();
  }
  lv_keyboard_set_textarea(this->kb, textarea);
  lv_obj_add_event_cb(textarea, ta_kb_event_cb, LV_EVENT_FOCUSED, this->kb);
  lv_obj_add_event_cb(textarea, ta_kb_event_cb, LV_EVENT_DEFOCUSED, this->kb);
}

void Display::flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
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
    ledcWrite(5, this->backlight_brightness); // Backlight on
  }
  // Update LVGL UI
  lv_timer_handler();
}

void Display::update_battery(int percentage, bool isCharging, bool isConnected)
{
  if (isConnected)
  {
    lv_label_set_text(this->objUSBIcon, LV_SYMBOL_USB);
  }
  else
  {
    lv_label_set_text(this->objUSBIcon, "");
  }
  // if(isCharging || !isConnected){
  //   Serial.println("charging?");
  //   lv_label_set_text(this->objBattPercentage, "");
  lv_label_set_text(this->objBattPercentage, String(percentage).c_str());
  //   lv_label_set_text(this->objBattIcon, LV_SYMBOL_USB);
  // }
  // else {
  //  Serial.println("Running on battery");
  //this->drawBattery(this->objBattIcon);
  if (percentage > 95)
  {
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_FULL);
  }
  else if (percentage > 75)
  {
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_3);
  }
  else if (percentage > 50)
  {
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_2);
  }
  else if (percentage > 25)
  {
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_1);
  }
  else
  {
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
  }
  //}
}

void Display::updateWifi(String symbol)
{
  lv_label_set_text(this->WifiLabel, symbol.c_str());
}

void Display::setActiveTab(byte tab)
{
  lv_tabview_set_act(display.getTabView(), tab, LV_ANIM_OFF);
}

lv_obj_t *Display::addTab(AppInterface* app)
{
  lv_obj_t *tab = nullptr;
  /* search free slot in tab array */
  for (int i = 0; i < TAB_ARRAY_SIZE; i++)
  {
    if (this->apps[i] == nullptr)
    {
      //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
      tab = lv_tabview_add_tab(this->tabview, app->getName().c_str());
      this->apps[i] = app;
      this->createTabviewButtons();

      // Initialize scroll position for the indicator
      //lv_event_send(lv_tabview_get_content(this->tabview), LV_EVENT_SCROLL, NULL);
      break;
    }
  }

  return tab;
}

AppInterface* Display::getApp(byte tab){
  return this->apps[tab];
}

void Display::createTabviewButtons()
{
  if (panel == nullptr)
  {
    //  Create a page indicator
    panel = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_CLICKABLE); // This indicator will not be clickable
    lv_obj_set_size(panel, this->width, 30);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
  }
  // delete old panel objects
  lv_obj_clean(panel);

  // This small hidden button enables the page indicator to scroll further
  lv_obj_t *btn = lv_btn_create(panel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

  // Create actual (non-clickable) buttons for every tab
  for (int i = 0; i < TAB_ARRAY_SIZE; i++)
  {
    if (this->apps[i] != nullptr)
    {
      AppInterface* app = this->apps[i];
      btn = lv_btn_create(panel);
      lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_size(btn, 150, lv_pct(100));
      lv_obj_t *label = lv_label_create(btn);
      lv_label_set_text_fmt(label, app->getName().c_str());
      lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
      lv_obj_set_style_bg_color(btn, this->primary_color, LV_PART_MAIN);
    }
  }

  // This small hidden button enables the page indicator to scroll further
  btn = lv_btn_create(panel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

  // Style the panel background
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_pad_all(&style_btn, 3);
  lv_style_set_border_width(&style_btn, 0);
  lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
  lv_obj_add_style(panel, &style_btn, 0);
}
