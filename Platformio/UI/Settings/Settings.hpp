#pragma once
#include "OmoteUI.hpp"
#include "HardwareAbstract.hpp"
#include "DisplayAbstract.h"
//#include <WiFi.h>
#include <DeviceInterface.hpp>
#include <AppInterface.hpp>
#include <string>

#define DEVICESLOTS 10
#define APPSLOTS 10

class Settings : public AppInterface
{
public:
    static HardwareAbstract* mHardware;
    static Settings* mInstance;
    static std::shared_ptr<Settings> getInstance() {return std::shared_ptr<Settings>(Settings::mInstance);}
    Settings(std::shared_ptr<DisplayAbstract> display);
    std::string getName();
    void handleCustomKeypad(int keyCode, char keyChar){};

    //bool addDevice(DeviceInterface* device);

    //bool addApp(AppInterface* app);

    void reset_settings_menu();
    void reset_wifi_menu();

    void displaySettings(lv_obj_t* parent) {};

    /**
     * @brief API function to inform display that wifi scan is completed
     *
     * @param size number of wifi networks found
     */
    void wifi_scan_complete(unsigned int size);

    /**
     * @brief Clear the wifi networks listed in the wifi selection page. This function is called before new wifi
     * networks are added to the list to avoid double listing
     *
     */
    void clear_wifi_networks();

    /**
     * @brief Update the wifi status. This function will update the wifi label in the status bar according to the
     * parameter.
     *
     * @param connected Boolean parameter to indicate if a wifi connection is established or not.
     */
    void update_wifi(bool connected);

    void saveSettings();

    bool wifiEnabled();

private:
    static void WifiEnableSetting_event_cb(lv_event_t *e);
    static void ta_event_cb(lv_event_t *e);
    static void WakeEnableSetting_event_cb(lv_event_t *e);
    static void IREnableSetting_event_cb(lv_event_t *e);
    static void to_dropdown_event_cb(lv_event_t *e);
    static void wifi_selected_cb(lv_event_t *e);
    static void wifi_settings_cb(lv_event_t *event);
    static void connect_btn_cb(lv_event_t *event);
    static void show_password_cb(lv_event_t *e);
    static bool wifiEnable;

    void setup();

    std::shared_ptr<DisplayAbstract> mDisplay;
    lv_obj_t *mTab;
    lv_obj_t *mSettingsMenu;
    lv_obj_t *mSettingsMainPage;

    void setup_settings(lv_obj_t *parent);
    void display_settings(lv_obj_t *parent);
    void ir_settings(lv_obj_t *parent);

    lv_obj_t *wifi_setting_cont;
    lv_obj_t *wifiOverview;
    lv_obj_t *wifi_password_label;
    lv_obj_t *wifi_password_page;
    lv_obj_t *wifi_selection_page;
    lv_obj_t *WifiLabel;
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

    lv_obj_t *ssidLabel = nullptr;
    lv_obj_t *ipLabel = nullptr;

    void factoryReset();

};
