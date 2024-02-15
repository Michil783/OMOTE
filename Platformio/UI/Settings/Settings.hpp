#pragma once
#include "OmoteUI.hpp"
#include "HardwareAbstract.hpp"
#include "DisplayAbstract.h"
//#include <WiFi.h>
#include <DeviceInterface.hpp>
#include <AppInterface.hpp>
#include <string>

class Settings : public AppInterface
{
public:
    static std::shared_ptr<Settings> getInstance() {return std::shared_ptr<Settings>(Settings::mInstance);}
    Settings(std::shared_ptr<DisplayAbstract> display);

    /* App Interface */
    //void addAppSettings(std::shared_ptr<void> settings) override {};
    void handleCustomKeypad(int keyCode, char keyChar) override {};
    std::string getName() override { return "Settings"; };
    void displaySettings(lv_obj_t *parent) override {};
    void saveSettings() override;

    void reset_settings_menu();
    void reset_wifi_menu();
    void wifi_scan_complete(unsigned int size);
    void clear_wifi_networks();
    void update_wifi(bool connected);
    bool isWifiEnabled();

private:
    static HardwareAbstract* mHardware;
    static Settings* mInstance;
    static std::shared_ptr<UI::Basic::OmoteUI> mOmoteUI;

    static void wifiEnableSetting_event_cb(lv_event_t *e);
    static void ta_event_cb(lv_event_t *e);
    static void IREnableSetting_event_cb(lv_event_t *e);
    static void to_dropdown_event_cb(lv_event_t *e);
    static void wakeEnableSetting_event_cb(lv_event_t *e);
    static void wifi_selected_cb(lv_event_t *e);
    static void wifi_settings_cb(lv_event_t *event);
    static void connect_btn_cb(lv_event_t *event);
    static void show_password_cb(lv_event_t *e);

    void setup();

    std::shared_ptr<DisplayAbstract> mDisplay;
    lv_obj_t *mTab;
    lv_obj_t *mSettingsMenu;
    lv_obj_t *mSettingsMainPage;

    void setup_settings(lv_obj_t *parent);
    void display_settings(lv_obj_t *parent);
    void ir_settings(lv_obj_t *parent);

    static bool mWifiEnabled;
    lv_obj_t *mWifiEnableSwitch;
    lv_obj_t *mWifiSettingsContent;
    lv_obj_t *mWifiOverview;
    lv_obj_t *mWifiPasswordLabel;
    lv_obj_t *mWifiPasswordPage;
    lv_obj_t *mWifiSelectionPage;
    lv_obj_t *mWifiLabel;
    unsigned int no_subpages;
    unsigned int no_wifi_networks;

    void next_wifi_selection_subpage(lv_event_t *e);
    lv_obj_t *create_wifi_selection_page(lv_obj_t *menu);
    lv_obj_t *create_wifi_password_page(lv_obj_t *menu);
    void create_wifi_main_page(lv_obj_t *parent);
    void create_wifi_settings(lv_obj_t *menu, lv_obj_t *parent);
    void update_wifi_selection_subpage(int page);

    //DeviceInterface* devices[DEVICESLOTS];
    //lv_obj_t* deviceOverview[DEVICESLOTS];
    void createDeviceSettings(lv_obj_t *menu, lv_obj_t *parent);

    //AppInterface* apps[APPSLOTS];
    //lv_obj_t* appOverview[APPSLOTS];
    void createAppSettings(lv_obj_t *menu, lv_obj_t *parent);

    void saveAppSettings();
    void saveDeviceSettings();

    lv_obj_t *mSSIDLabel = nullptr;
    lv_obj_t *mIPLabel = nullptr;

    void factoryReset();

};
