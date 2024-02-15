#include "OmoteUI.hpp"
#include "lvgl.h"
#include "omoteconfig.h"
#include <functional>
#include "HardwareFactory.hpp"
#include "HardwareAbstract.hpp"

using namespace UI::Basic;

std::shared_ptr<OmoteUI> OmoteUI::mInstance = nullptr;
//std::shared_ptr<HardwareAbstract> OmoteUI::mHardware = nullptr;
lv_obj_t *OmoteUI::mPanel = nullptr;
uint_fast8_t OmoteUI::mCurrentDevice = 1;

  //Images imgs = Images();
  static uint_fast8_t currentDevice;

OmoteUI::OmoteUI() : UIBase() {
    /*Initialize tab name array*/
  for (int i = 0; i < APPSLOTS; i++)
  {
    mApps[i] = nullptr;
  }
  mPanel =
  mStatusbar =
  mWifiLabel =
  mBattPercentage =
  mBattIcon =
  mUSBIcon = nullptr;
  currentDevice = 1;
  mBatteryChargingIndex = 0;
  mInstance = std::shared_ptr<UI::Basic::OmoteUI>(this);

  setup_ui();
  //mHardware = std::shared_ptr<HardwareAbstract> (&HardwareFactory::getAbstract());
};

// Set the page indicator scroll position relative to the tabview scroll
// position
void OmoteUI::store_scroll_value_event_cb(lv_event_t *e) {
  float bias = (150.0 + 8.0) / 240.0;
  int offset = 240 / 2 - 150 / 2 - 8 - 50 - 3;
  lv_obj_t *screen = lv_event_get_target(e);
  lv_obj_scroll_to_x(mPanel, lv_obj_get_scroll_x(screen) * bias - offset,
                     LV_ANIM_OFF);
}

// Update current device when the tabview page is changes
void OmoteUI::tabview_device_event_cb(lv_event_t *e) {
  currentDevice = lv_tabview_get_tab_act(lv_event_get_target(e));
  //mHardware->setCurrentDevice(currentDevice);
}

void OmoteUI::onPollCb(){
  getInstance()->update_battery((HardwareFactory::getAbstract()).battery()->getPercentage(),
     (HardwareFactory::getAbstract()).battery()->isCharging(),
     (HardwareFactory::getAbstract()).battery()->isConnected());
}

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

  mWifiLabel = lv_label_create(statusbar);
  lv_label_set_text(mWifiLabel, "");
  lv_obj_align(mWifiLabel, LV_ALIGN_LEFT_MID, -8, 0);
  lv_obj_set_style_text_font(mWifiLabel, &lv_font_montserrat_12,
                             LV_PART_MAIN);

  mBattPercentage = lv_label_create(statusbar);
  lv_label_set_text(mBattPercentage, "");
  lv_obj_align(mBattPercentage, LV_ALIGN_RIGHT_MID, -16, 0);
  lv_obj_set_style_text_font(mBattPercentage, &lv_font_montserrat_12,
                             LV_PART_MAIN);

  mBattIcon = lv_label_create(statusbar);
  lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_align(mBattIcon, LV_ALIGN_RIGHT_MID, 8, 0);
  lv_obj_set_style_text_font(mBattIcon, &lv_font_montserrat_16,
                             LV_PART_MAIN);

  mUSBIcon = lv_label_create(statusbar);
  lv_label_set_text(mUSBIcon, LV_SYMBOL_USB);
  lv_obj_align(mUSBIcon, LV_ALIGN_RIGHT_MID, -40, 0);
  lv_obj_set_style_text_font(mUSBIcon, &lv_font_montserrat_16, LV_PART_MAIN);

  LV_LOG_USER("create poller");
  mBatteryPoller = std::make_unique<poller>(onPollCb, ((std::chrono::seconds)(1)));
}

/* Callback function to show and hide keybaord for attached textareas */
void OmoteUI::ta_kb_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *mKb = (lv_obj_t *)lv_event_get_user_data(e);
  switch (code) {
  case LV_EVENT_FOCUSED:
    lv_keyboard_set_textarea(mKb, ta);
    lv_obj_clear_flag(mKb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(mKb);
    break;
  case LV_EVENT_DEFOCUSED:
    lv_keyboard_set_textarea(mKb, NULL);
    lv_obj_add_flag(mKb, LV_OBJ_FLAG_HIDDEN);
    break;
  default:
    break;
  }
}

void OmoteUI::create_keyboard() {
  mKb = lv_keyboard_create(lv_scr_act());
  lv_obj_add_flag(mKb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_y(mKb, 0);
}

void OmoteUI::attach_keyboard(lv_obj_t *textarea)
{
  if (mKb == NULL)
  {
    create_keyboard();
  }
  lv_keyboard_set_textarea(mKb, textarea);
  lv_obj_add_event_cb(textarea, (lv_event_cb_t)ta_kb_event_cb, LV_EVENT_FOCUSED, mKb);
  lv_obj_add_event_cb(textarea, (lv_event_cb_t)ta_kb_event_cb, LV_EVENT_DEFOCUSED, mKb);
}

void OmoteUI::hide_keyboard() { 
  lv_obj_add_flag(mKb, LV_OBJ_FLAG_HIDDEN);
}

    void OmoteUI::show_keyboard()
    {
        lv_obj_clear_flag(mKb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(mKb);
    };

lv_obj_t *OmoteUI::addTab(AppInterface* app)
{
  LV_LOG_USER(">>> OmoteUI::addTab(%s)", app->getName().c_str());
  lv_obj_t *tab = nullptr;
  /* search free slot in tab array */
  for (int i = 0; i < APPSLOTS; i++)
  {
    if (mApps[i] == nullptr)
    {
      LV_LOG_USER("adding app");
      //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
      tab = lv_tabview_add_tab(mTabView, app->getName().c_str());
      mApps[i] = app;
      createTabviewButtons();

      // Initialize scroll position for the indicator
      lv_event_send(lv_tabview_get_content(mTabView), LV_EVENT_SCROLL, NULL);
      LV_LOG_USER("<<< OmoteUI::addTab(%s)", app->getName().c_str());
      return tab;
    }
  }
  LV_LOG_USER("<<< OmoteUI::addTab(%s)", app->getName().c_str());
  LV_LOG_ERROR("no free App slot");
  return nullptr;
}

void OmoteUI::createTabviewButtons()
{
  if (mPanel == nullptr)
  {
    //  Create a page indicator
    mPanel = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(mPanel, LV_OBJ_FLAG_CLICKABLE); // This indicator will not be clickable
    lv_obj_set_size(mPanel, SCREEN_WIDTH, 30);
    lv_obj_set_flex_flow(mPanel, LV_FLEX_FLOW_ROW);
    lv_obj_align(mPanel, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(mPanel, LV_SCROLLBAR_MODE_OFF);
  }
  // delete old panel objects
  lv_obj_clean(mPanel);

  // This small hidden button enables the page indicator to scroll further
  lv_obj_t *btn = lv_btn_create(mPanel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

  // Create actual (non-clickable) buttons for every tab
  for (int i = 0; i < APPSLOTS; i++)
  {
    if (mApps[i] != nullptr)
    {
      AppInterface* app = mApps[i];
      btn = lv_btn_create(mPanel);
      lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_size(btn, 150, lv_pct(100));
      lv_obj_t *label = lv_label_create(btn);
      lv_label_set_text(label, app->getName().c_str());
      lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
      lv_obj_set_style_bg_color(btn, mPrimaryColor, LV_PART_MAIN);
    }
  }

  // This small hidden button enables the page indicator to scroll further
  btn = lv_btn_create(mPanel);
  lv_obj_set_size(btn, 50, lv_pct(100));
  lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
  lv_obj_set_style_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);

  // Style the panel background
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_pad_all(&style_btn, 3);
  lv_style_set_border_width(&style_btn, 0);
  lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
  lv_obj_add_style(mPanel, &style_btn, 0);
}

void OmoteUI::setup_ui()
{
  LV_LOG_TRACE(">>> OmoteUI::setup_ui()");

  // Set the background color
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

  create_keyboard();
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

  create_status_bar();
  LV_LOG_USER("<<< OmoteUI::setup_ui()");
}

void OmoteUI::update_battery(int percentage, bool isCharging, bool isConnected)
{
  if (isConnected)
  {
    lv_label_set_text(mUSBIcon, LV_SYMBOL_USB);
  }
  else
  {
    lv_label_set_text(mUSBIcon, "");
  }
  lv_label_set_text(mBattPercentage, std::to_string(percentage).c_str());
  if( isCharging ) {
    LV_LOG_TRACE("isCharging: mBatteryChargingIndex=%d", mBatteryChargingIndex);
    lv_label_set_text(mBattIcon, mBatteryCharging[mBatteryChargingIndex++]);
    if( mBatteryChargingIndex == BATTERYCHARGINGINDEX_MAX ) mBatteryChargingIndex = 0;
  }
  else {
    if (percentage > 95)
    {
      lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_FULL);
    }
    else if (percentage > 75)
    {
      lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_3);
    }
    else if (percentage > 50)
    {
      lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_2);
    }
    else if (percentage > 25)
    {
      lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_1);
    }
    else
    {
      lv_label_set_text(mBattIcon, LV_SYMBOL_BATTERY_EMPTY);
    }
  }
}

