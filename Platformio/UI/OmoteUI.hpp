// OMOTE UI
// 2023 Matthew Colvin
#pragma once

#include "wifiHandlerInterface.h"
#include "UIBase.hpp"
#include "lvgl.h"
#include "poller.hpp"
#include "Images.hpp"
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <string>

#include "AppInterface.hpp"
#include "DeviceInterface.hpp"

namespace UI::Basic
{
  /// @brief Singleton to allow UI code to live separately from the Initialization
  /// of resources.
  class OmoteUI : public UIBase
  {
  public:
    // OmoteUI(std::shared_ptr<HardwareAbstract> aHardware);
    OmoteUI();

    static std::weak_ptr<OmoteUI> getRefrence() { return getInstance(); };
    static std::shared_ptr<OmoteUI>
    getInstance(std::shared_ptr<HardwareAbstract> aHardware = nullptr)
    {
      if (mInstance)
      {
        return mInstance;
      }
      else if (aHardware)
      {
        // mInstance = std::make_shared<OmoteUI>(aHardware);
        mInstance = std::make_shared<OmoteUI>();
      }
      return mInstance;
    };

    // Set the page indicator scroll position relative to the tabview scroll
    // position
    static void store_scroll_value_event_cb(lv_event_t *e);
    // Update current device when the tabview page is changes
    static void tabview_device_event_cb(lv_event_t *e);
    // // Update wake timeout handler
    // void wakeTimeoutSetting_event_cb(lv_event_t *e);
    // // Slider Event handler
    // void bl_slider_event_cb(lv_event_t *e);
    // // Apple Key Event handler
    // void appleKey_event_cb(lv_event_t *e);
    // // Wakeup by IMU Switch Event handler
    // void WakeEnableSetting_event_cb(lv_event_t *e);
    // // Smart Home Toggle Event handler
    // void smartHomeToggle_event_cb(lv_event_t *e);
    // // Smart Home Toggle Event handler
    // void smartHomeSlider_event_cb(lv_event_t *e);
    // // Virtual Keypad Event handler
    void virtualKeypad_event_cb(lv_event_t *e);
    void wifi_settings_cb(lv_event_t *event);

    void connect_btn_cb(lv_event_t *event);

    void password_field_event_cb(lv_event_t *e);

    static void ta_kb_event_cb(lv_event_t *e);

    void wifi_scan_done(std::shared_ptr<std::vector<wifiHandlerInterface::WifiInfo>> info);
    void loopHandler();

    void hide_keyboard();
    void show_keyboard();

    void create_keyboard();
    void attach_keyboard(lv_obj_t *textarea);

    void setup_ui();

    lv_color_t getPrimaryColor() { return mPrimaryColor; };
    lv_obj_t *addApp(AppInterface *app);
    lv_obj_t *getAppView() { return mTabView; };
    lv_obj_t *addDevice(DeviceInterface *device);
    void updateWifi(std::string symbol) { lv_label_set_text(mWifiLabel, symbol.c_str()); };
    void setActiveTab(unsigned char tab) { lv_tabview_set_act(getAppView(), tab, LV_ANIM_OFF); };
    AppInterface *getApp(unsigned char tab) { return mApps[tab]; };
    DeviceInterface *getDevice(unsigned char tab) { return mDevices[tab]; };
    void update_battery(int percentage, bool isCharging, bool isConnected);

  private:
    static void onPollCb();

    //static std::shared_ptr<HardwareAbstract> mHardware;
    static std::shared_ptr<OmoteUI> mInstance;
    static lv_obj_t *mPanel;
    static uint_fast8_t mCurrentDevice;

    std::unique_ptr<poller> mBatteryPoller;

    int mSleepTimeoutMap[5] = {10000, 30000, 60000, 180000, 600000};

    std::shared_ptr<std::vector<wifiHandlerInterface::WifiInfo>> found_wifi_networks;

    lv_obj_t *mKb;

    // static lv_obj_t *panel;
    // //Images imgs = Images();
    // static uint_fast8_t currentDevice;

    lv_color_t mPrimaryColor = lv_color_hex(0x303030); // gray

    // inline static const uint_fast8_t virtualKeyMapTechnisat[10] = {
    //     0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x0};

    /************************************** WIFI Settings Menu
     * *******************************************************/
    // lv_obj_t *mWifi_setting_cont;
    // lv_obj_t *mWifiOverview;
    // lv_obj_t *mWifi_password_label;
    // lv_obj_t *mWifi_password_page;
    // lv_obj_t *mWifi_selection_page;
    // unsigned int no_subpages;
    // unsigned int no_wifi_networks;

    // void wifi_status(std::shared_ptr<wifiHandlerInterface::wifiStatus> status);
    // void next_wifi_selection_subpage(lv_event_t *e);
    // lv_obj_t *create_wifi_selection_page(lv_obj_t *menu);
    // lv_obj_t *create_wifi_password_page(lv_obj_t *menu);
    // void create_wifi_main_page(lv_obj_t *parent);
    // void create_wifi_settings(lv_obj_t *menu, lv_obj_t *parent);
    // void update_wifi_selection_subpage(int page);

    // void display_settings(lv_obj_t *parent);

    AppInterface *mApps[APPSLOTS];
    lv_obj_t *mTabView;
    void createTabviewButtons();
    // void setup_settings(lv_obj_t *parent);
    // lv_obj_t *mSettingsMenu;
    DeviceInterface *mDevices[DEVICESLOTS];

    /******************************************** Statusbar *************************************************************/
    void create_status_bar();

    lv_obj_t *mStatusbar;
    lv_obj_t *mWifiLabel;
    lv_obj_t *mBattPercentage;
    lv_obj_t *mBattIcon;
    lv_obj_t *mUSBIcon;

    int mBatteryChargingIndex = 0;
    const char *mBatteryCharging[BATTERYCHARGINGINDEX_MAX] = {LV_SYMBOL_BATTERY_EMPTY, LV_SYMBOL_BATTERY_1, LV_SYMBOL_BATTERY_2, LV_SYMBOL_BATTERY_3, LV_SYMBOL_BATTERY_FULL};
  };

} // namespace UI::Basic