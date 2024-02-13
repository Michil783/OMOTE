#include "OmoteUI.hpp"
#include "lvgl.h"
#include "omoteconfig.h"
#include <functional>

using namespace UI::Basic;

std::shared_ptr<OmoteUI> OmoteUI::mInstance = nullptr;

OmoteUI::OmoteUI() : UIBase() {
    /*Initialize tab name array*/
  for (int i = 0; i < TAB_ARRAY_SIZE; i++)
  {
    mApps[i] = nullptr;
  }
  panel = nullptr;
  currentDevice = 1;
};

// Set the page indicator scroll position relative to the tabview scroll
// position
void OmoteUI::store_scroll_value_event_cb(lv_event_t *e) {
  float bias = (150.0 + 8.0) / 240.0;
  int offset = 240 / 2 - 150 / 2 - 8 - 50 - 3;
  lv_obj_t *screen = lv_event_get_target(e);
  lv_obj_scroll_to_x(panel, lv_obj_get_scroll_x(screen) * bias - offset,
                     LV_ANIM_OFF);
}

// Update current device when the tabview page is changes
void OmoteUI::tabview_device_event_cb(lv_event_t *e) {
  currentDevice = lv_tabview_get_tab_act(lv_event_get_target(e));
  mHardware->setCurrentDevice(currentDevice);
}

// // Slider Event handler
// void OmoteUI::bl_slider_event_cb(lv_event_t *e) {
//   lv_obj_t *slider = lv_event_get_target(e);
//   auto newBrightness = std::clamp(lv_slider_get_value(slider), 60, 255);
//   mHardware->display()->setBrightness(newBrightness);
// }

// // Apple Key Event handler
// void OmoteUI::appleKey_event_cb(lv_event_t *e) {
//   // Send IR command based on the event user data
//   // mHardware->debugPrint(std::to_string(50 + (int)e->user_data));
// }

// // Wakeup by IMU Switch Event handler
// void OmoteUI::WakeEnableSetting_event_cb(lv_event_t *e) {
//   this->mHardware->setWakeupByIMUEnabled(
//       lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
// }

// // Wakeup timeout dropdown Event handler
// void OmoteUI::wakeTimeoutSetting_event_cb(lv_event_t *e) {
//   lv_obj_t *drop = lv_event_get_target(e);

//   int sleepTimeout = sleepTimeoutMap[lv_dropdown_get_selected(drop)];
//   mHardware->setSleepTimeout(sleepTimeout);
// }

// // Smart Home Toggle Event handler
// void OmoteUI::smartHomeToggle_event_cb(lv_event_t *e) {
//   char payload[8];
//   if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED))
//     strcpy(payload, "true");
//   else
//     strcpy(payload, "false");
//   // Publish an MQTT message based on the event user data
//   // if ((int)e->user_data == 1)
//   //   mHardware->MQTTPublish("bulb1_set", payload);
//   // if ((int)e->user_data == 2)
//   //   mHardware->MQTTPublish("bulb2_set", payload);
// }

// // Smart Home Toggle Event handler
// void OmoteUI::smartHomeSlider_event_cb(lv_event_t *e) {
//   lv_obj_t *slider = lv_event_get_target(e);
//   char payload[8];
//   auto sliderValue = lv_slider_get_value(slider);

//   // TODO convert this dtostrf to somethign more portable.
//   //  I gave it a stab here but not sure it is the same.
//   // dtostrf(lv_slider_get_value(slider), 1, 2, payload);
//   snprintf(payload, sizeof(payload), "%8.2d", sliderValue);

//   // Publish an MQTT message based on the event user data
//   // if ((int)e->user_data == 1)
//   //   mHardware->MQTTPublish("bulb1_setbrightness", payload);
//   // if ((int)e->user_data == 2)
//   //   mHardware->MQTTPublish("bulb2_setbrightness", payload);
// }

void OmoteUI::virtualKeypad_event_cb(lv_event_t *e) {
  lv_obj_t *target = lv_event_get_target(e);
  lv_obj_t *cont = lv_event_get_current_target(e);
  if (target == cont)
    return;
}

void OmoteUI::loopHandler() {
  lv_timer_handler();
  lv_task_handler();
}

void OmoteUI::create_status_bar() {
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
  lv_obj_set_style_text_font(this->WifiLabel, &lv_font_montserrat_12,
                             LV_PART_MAIN);

  this->objBattPercentage = lv_label_create(statusbar);
  lv_label_set_text(this->objBattPercentage, "");
  lv_obj_align(this->objBattPercentage, LV_ALIGN_RIGHT_MID, -16, 0);
  lv_obj_set_style_text_font(this->objBattPercentage, &lv_font_montserrat_12,
                             LV_PART_MAIN);

  this->objBattIcon = lv_label_create(statusbar);
  lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_align(this->objBattIcon, LV_ALIGN_RIGHT_MID, 8, 0);
  lv_obj_set_style_text_font(this->objBattIcon, &lv_font_montserrat_16,
                             LV_PART_MAIN);

  batteryPoller = std::make_unique<poller>(
      [&batteryIcon = objBattIcon, battery = mHardware->battery()]() {
        auto percent = battery->getPercentage();
        if (percent > 95)
          lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_FULL);
        else if (percent > 75)
          lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_3);
        else if (percent > 50)
          lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_2);
        else if (percent > 25)
          lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_1);
        else
          lv_label_set_text(batteryIcon, LV_SYMBOL_BATTERY_EMPTY);
      });
}

/* Callback function to show and hide keybaord for attached textareas */
void OmoteUI::ta_kb_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  switch (code) {
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

void OmoteUI::create_keyboard() {
  kb = lv_keyboard_create(lv_scr_act());
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_y(kb, 0);
}

void OmoteUI::attach_keyboard(lv_obj_t *textarea)
{
  if (kb == NULL)
  {
    create_keyboard();
  }
  lv_keyboard_set_textarea(kb, textarea);
  lv_obj_add_event_cb(textarea, (lv_event_cb_t)ta_kb_event_cb, LV_EVENT_FOCUSED, kb);
  lv_obj_add_event_cb(textarea, (lv_event_cb_t)ta_kb_event_cb, LV_EVENT_DEFOCUSED, kb);
}

void OmoteUI::hide_keyboard() { 
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

    void OmoteUI::show_keyboard()
    {
        lv_obj_clear_flag(this->kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(this->kb);
    };

lv_obj_t *OmoteUI::addTab(AppInterface* app)
{
  lv_obj_t *tab = nullptr;
  /* search free slot in tab array */
  for (int i = 0; i < TAB_ARRAY_SIZE; i++)
  {
    if (mApps[i] == nullptr)
    {
      //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
      tab = lv_tabview_add_tab(mTabView, app->getName().c_str());
      mApps[i] = app;
      this->createTabviewButtons();

      // Initialize scroll position for the indicator
      //lv_event_send(lv_tabview_get_content(mTabView), LV_EVENT_SCROLL, NULL);
      break;
    }
  }

  return tab;
}

void OmoteUI::createTabviewButtons()
{
  if (panel == nullptr)
  {
    //  Create a page indicator
    panel = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_CLICKABLE); // This indicator will not be clickable
    lv_obj_set_size(panel, SCREEN_WIDTH, 30);
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
    if (mApps[i] != nullptr)
    {
      AppInterface* app = mApps[i];
      btn = lv_btn_create(panel);
      lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_size(btn, 150, lv_pct(100));
      lv_obj_t *label = lv_label_create(btn);
      lv_label_set_text_fmt(label, app->getName().c_str());
      lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
      lv_obj_set_style_bg_color(btn, mPrimaryColor, LV_PART_MAIN);
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

void OmoteUI::setup_ui()
{
  LV_LOG_TRACE("Display::setup_ui()");

  // Set the background color
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

  this->create_keyboard();
  // Setup a scrollable tabview for devices and settings
  mTabView = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 0); // Hide tab labels by setting their height to 0
  lv_obj_set_style_bg_color(mTabView, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_size(mTabView, SCREEN_WIDTH, 270); // 270 = screenHeight(320) - panel(30) - statusbar(20)
  lv_obj_align(mTabView, LV_ALIGN_TOP_MID, 0, 20);

  // Configure number button grid
  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
  static lv_coord_t row_dsc[] = {52, 52, 52, 52, LV_GRID_TEMPLATE_LAST};                              // manual y distribution to compress the grid a bit

  // Set current page according to the current Device
  lv_tabview_set_act(mTabView, 0, LV_ANIM_OFF);

  // Make the indicator scroll together with the tabs by creating a scroll event
  lv_obj_add_event_cb(lv_tabview_get_content(mTabView), (lv_event_cb_t)store_scroll_value_event_cb, LV_EVENT_SCROLL, NULL);
  lv_obj_add_event_cb(mTabView, (lv_event_cb_t)tabview_device_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  // Initialize scroll position for the indicator
  //lv_event_send(lv_tabview_get_content(mTabView), LV_EVENT_SCROLL, NULL);

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

void OmoteUI::update_battery(int percentage, bool isCharging, bool isConnected)
{
  if (isConnected)
  {
    lv_label_set_text(this->objUSBIcon, LV_SYMBOL_USB);
  }
  else
  {
    lv_label_set_text(this->objUSBIcon, "");
  }
  lv_label_set_text(this->objBattPercentage, std::to_string(percentage).c_str());
  if( isCharging ) {
    lv_label_set_text(this->objBattIcon, this->batteryCharging[this->batteryChargingIndex++]);
    if( this->batteryChargingIndex == BATTERYCHARGINGINDEX_MAX ) this->batteryChargingIndex = 0;
  }
  else {
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
  }
}

