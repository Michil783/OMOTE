#include <Settings.hpp>
#include "HardwareFactory.hpp"
#include <wifiHandlerInterface.h>
#include "OmoteUI.hpp"
#include "omoteconfig.h"
//#include <IRHandler.hpp>
#ifdef OMOTE_ESP32
#include <Preferences.h>
#endif

#define WIFI_SUBPAGE_SIZE 3

LV_IMG_DECLARE(high_brightness);
LV_IMG_DECLARE(low_brightness);
LV_IMG_DECLARE(WiFi_No_Signal);
LV_IMG_DECLARE(WiFi_Low_Signal);
LV_IMG_DECLARE(WiFi_Mid_Signal);
LV_IMG_DECLARE(WiFi_High_Signal);

//extern bool wakeupByIMUEnabled;
//extern long standbyTimerConfigured;
static char *ssid;

//extern IRHandler irhandler;
//extern Preferences preferences;

//#include "Display.hpp"

/**
 * @brief Textarea callback function for the password field. In case the enter key is pressed in the text area, the
 * function will try to connect to the network with the provided password.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

    const char *password = lv_textarea_get_text(ta);
    switch (code)
    {
    case LV_EVENT_READY:
        Settings::mHardware->wifi()->connect(ssid, password);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        mOmoteUI->hide_keyboard();
        getInstance()->reset_settings_menu();
        /* Fall through on purpose. Pressing enter should disable the keyboard as well*/
    default:
        break;
    }
}

/**
 * @brief Checkbox callback function for the lift to wake option
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::wakeEnableSetting_event_cb(lv_event_t *e)
{
    // LV_LOG_TRACE("Settings - WakeEnableSetting_event_cb");
    Settings::mHardware->setWakeupByIMUEnabled(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

/**
 * @brief Checkbox callback function for the IR receiver option
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::IREnableSetting_event_cb(lv_event_t *e)
{
    LV_LOG_TRACE("Settings - IREnableSetting_event_cb");
    LV_LOG_USER("IREnableSetting_event_cb: state=%d", lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
    Settings::mHardware->irhandler()->IRReceiverEnable(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

/**
 * @brief Slider callback function for the backlight brightness.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void bl_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    unsigned int *backlight_brightness = (unsigned int *)lv_event_get_user_data(e);
    #ifdef OMOTE_ESP32
    *backlight_brightness = map(constrain(lv_slider_get_value(slider), 30, 240), 30, 240, 240, 30);
    #endif
    // LV_LOG_TRACE("Settings - bl_slider_event_cb(%d) - %d", constrain(lv_slider_get_value(slider), 30, 240), *backlight_brightness);
}

void Settings::wifiEnableSetting_event_cb(lv_event_t *e)
{
    LV_LOG_USER(">>> Settings - wifiEnableSetting_event_cb");
    LV_LOG_USER("mWifiOverview: %p", mInstance->mWifiOverview);
    mInstance->mWifiEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("Settings", false);
    preferences.putBool("mWifiEnabled", mWifiEnabled);
    #endif
    LV_LOG_USER("mWifiEnabled: %d", mWifiEnabled);
    mInstance->reset_wifi_menu();
    #ifdef ENABLE_WIFI
    LV_LOG_USER("mWifiEnabled: %d wifiConnected: %d", mInstance->isWifiEnabled(), mHardware->wifi()->GetStatus().isConnected);
    if( mInstance->isWifiEnabled() ){
        LV_LOG_USER("mInstance->isWifiEabled() = %d", mInstance->isWifiEnabled());
        mHardware->wifi()->begin();
    } else {
        LV_LOG_USER("mInstance->isWifiEnabled() = %d", mInstance->isWifiEnabled());
        mHardware->wifi()->turnOff();
        getInstance()->update_wifi(false);
        mOmoteUI->updateWifi("");
    }
    #endif
    LV_LOG_USER("mWifiOverview: %p", mInstance->mWifiOverview);
    LV_LOG_USER("<<< Settings - wifiEnableSetting_event_cb");
}

/**
 * @brief Dropdown list callback function for the screen timeout duration.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::to_dropdown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int index = lv_dropdown_get_selected(obj);
        // LV_LOG_TRACE("new value = %d\n", index);
        //std::shared_ptr<HardwareAbstract> hardware = lv_event_get_user_data(e);
        switch (index)
        {
        case 1:
            Settings::mHardware->setSleepTimeout(30000);
            break;
        case 2:
            Settings::mHardware->setSleepTimeout(60000);
            break;
        case 3:
            Settings::mHardware->setSleepTimeout(500000);
            break;
        default:
            Settings::mHardware->setSleepTimeout(10000);
            break;
        }
    }
}

/**
 * @brief Callback function in case a wifi is selected. This callback function will change the label of the wifi password
 * sub page to the selected wifi network.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::wifi_selected_cb(lv_event_t *e)
{
    lv_obj_t *label = lv_obj_get_child(e->target, 0);
    lv_label_set_text((lv_obj_t *)e->user_data, lv_label_get_text(label));
    ssid = lv_label_get_text(label);
}

/**
 * @brief Callback which is triggered when the wifi settings are opened (the wifi settings are pressed in the settings
 * main page). This function will trigger the asynchronous scan for wifi networks and update the label of the wifi
 * selection page to indicate that wifi networks are being searched for. The wifi event callback function then has to
 * call the API function to fill the page with the found networks.
 *
 * @param event Pointer to event object for the event where this callback is called
 */
void Settings::wifi_settings_cb(lv_event_t *event)
{
    lv_obj_t *cont = (lv_obj_t *)lv_event_get_user_data(event);
    lv_obj_clean(cont);
    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, "Searching for wifi networks");
    // This will trigger an asynchronouse network scan
    Settings::mHardware->wifi()->scan();
}

/**
 * @brief Callback which is triggered when clicking the connect button. It triggers the wifi connection.
 *
 * @param event Pointer to event object for the event where this callback is called
 */
void Settings::connect_btn_cb(lv_event_t *event)
{
    lv_obj_t *ta = (lv_obj_t *)event->user_data;
    const char *password = lv_textarea_get_text(ta);

    Settings::mHardware->wifi()->connect(ssid, password);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    //display.hide_keyboard();
    mOmoteUI->hide_keyboard();
    getInstance()->reset_settings_menu();
}

/**
 * @brief Callback function for the show password checkbox. Checking the box will show the password while unchecked the
 * password will be shown as dots.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void Settings::show_password_cb(lv_event_t *e)
{
    lv_obj_t *password_field = (lv_obj_t *)e->user_data;
    if (lv_obj_has_state(e->target, LV_STATE_CHECKED))
    {
        lv_textarea_set_password_mode(password_field, false);
    }
    else
    {
        lv_textarea_set_password_mode(password_field, true);
    }
}

lv_obj_t *wifi_subpage;

HardwareAbstract* Settings::mHardware;
Settings* Settings::mInstance;
bool Settings::mWifiEnabled;
std::shared_ptr<UI::Basic::OmoteUI> Settings::mOmoteUI;

Settings::Settings(std::shared_ptr<DisplayAbstract> display)
{
    LV_LOG_USER(">>> Settings::Settings()");
    mDisplay = display;
    mHardware = &HardwareFactory::getAbstract();
    mInstance = this;
    mOmoteUI = UI::Basic::OmoteUI::getInstance();

    /*Initialize device array*/
    // for (int i = 0; i < DEVICESLOTS; i++)
    // {
    //     this->devices[i] = nullptr;
    // }

    mWifiEnabled = true;
    mWifiSettingsContent =
    mWifiOverview =
    mWifiPasswordLabel =
    mWifiPasswordPage =
    mWifiSelectionPage =
    mWifiLabel = nullptr;

    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("Settings", false);
    mWifiEnabled = preferences.getBool("mWifiEnabled", false);
    #endif
    LV_LOG_USER("mWifiEnabled: %d", mWifiEnabled);

    mTab = mOmoteUI->addTab(this);
    setup();
    LV_LOG_USER("<<< Settings::Settings()");
}

void Settings::factoryReset(){
    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("Settings", false);
    preferences.clear();
    #endif
}

bool Settings::isWifiEnabled(){
    return mWifiEnabled;
}

/**
 * @brief setup of the settings screen
 * function will build display and Wifi setings areas
 *
 * @param
 */
void Settings::setup()
{
    LV_LOG_USER(">>> Settings::setup()");
    /* Create main page for settings mSettingsMenu*/
    if( mTab != nullptr ) {
        LV_LOG_USER("setup all Settings menu");
        setup_settings(mTab);
    }
    else
        LV_LOG_ERROR("no TebView pointer returned");
    LV_LOG_USER("<<< Settings::setup()");
}

/**
 * @brief build the settings in LVGL
 *
 * @param lv_obj_t* parent
 */
void Settings::setup_settings(lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::setup_settings()");
    // Add content to the settings mTab
    // With a flex layout, setting groups/boxes will position themselves automatically
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);
    // Add a label, then a box for the display settings
    mSettingsMenu = lv_menu_create(parent);
    lv_obj_set_width(mSettingsMenu, 210);

    /* Create main page for settings mSettingsMenu*/
    mSettingsMainPage = lv_menu_page_create(mSettingsMenu, NULL);
    lv_obj_t *cont = lv_menu_cont_create(mSettingsMainPage);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);

    display_settings(cont);
    ir_settings(cont);
    create_wifi_settings(mSettingsMenu, cont);
    createAppSettings(mSettingsMenu, cont);
    createDeviceSettings(mSettingsMenu, cont);

    lv_menu_set_page(mSettingsMenu, mSettingsMainPage);

    LV_LOG_USER("<<< Settings::setup_settings()");
}

/**
 * @brief display the settings
 *
 * @param lv_obj_t* parent
 */
void Settings::display_settings(lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::display_settings()");

    lv_color_t primary_color = mOmoteUI->getPrimaryColor();
    unsigned int *backlight_brightness = mDisplay->getBacklightBrightness();

    lv_obj_t *menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, "Display");

    lv_obj_t *menuBox = lv_obj_create(parent);
    lv_obj_set_size(menuBox, lv_pct(100), 109);
    lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

    lv_obj_t *brightnessIcon = lv_img_create(menuBox);
    lv_img_set_src(brightnessIcon, &low_brightness);
    lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(brightnessIcon, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *slider = lv_slider_create(menuBox);
    lv_slider_set_range(slider, 30, 240);
    lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
    #ifdef OMOTE_ESP32
    lv_slider_set_value(slider, map(*backlight_brightness, 240, 30, 30, 240), LV_ANIM_OFF);
    #else
    lv_slider_set_value(slider, 120, LV_ANIM_OFF);
    #endif
    //LV_LOG_TRACE("set blSlider to %d", map(*backlight_brightness, 240, 30, 30, 240));
    lv_obj_set_size(slider, lv_pct(66), 10);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 3);
    brightnessIcon = lv_img_create(menuBox);
    lv_img_set_src(brightnessIcon, &high_brightness);
    lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(brightnessIcon, LV_ALIGN_TOP_RIGHT, 0, -1);
    lv_obj_add_event_cb(slider, bl_slider_event_cb, LV_EVENT_VALUE_CHANGED, backlight_brightness);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "Lift to Wake");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 32);
    lv_obj_t *wakeToggle = lv_switch_create(menuBox);
    lv_obj_set_size(wakeToggle, 40, 22);
    lv_obj_align(wakeToggle, LV_ALIGN_TOP_RIGHT, 0, 29);
    lv_obj_set_style_bg_color(wakeToggle, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_add_event_cb(wakeToggle, wakeEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    if (Settings::Settings::mHardware->getWakeupByIMUEnabled())
        lv_obj_add_state(wakeToggle, LV_STATE_CHECKED); // set default state

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "Timeout");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 64);
    lv_obj_t *drop = lv_dropdown_create(menuBox);
    lv_dropdown_set_options(drop, "10s\n"
                                  "30s\n"
                                  "1m\n"
                                  "3m");
    lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, 61);
    lv_obj_set_size(drop, 70, 22);
    // lv_obj_set_style_text_font(drop, &lv_font_montserrat_12, LV_PART_MAIN);
    // lv_obj_set_style_text_font(lv_dropdown_get_list(drop), &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(drop, primary_color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);
    uint16_t selected = 0;
    switch (Settings::mHardware->getSleepTimeout())
    {
    case 30000:
        selected = 1;
        break;
    case 60000:
        selected = 2;
        break;
    case 500000:
        selected = 3;
        break;
    default:
        break;
    }
    lv_dropdown_set_selected(drop, selected);
    lv_obj_add_event_cb(drop, to_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, (void*)Settings::mHardware);
    LV_LOG_USER("<<< Settings::display_settings()");
}

void Settings::reset_settings_menu()
{
    LV_LOG_USER("");
    if( mSettingsMenu && mSettingsMainPage )
        lv_menu_set_page(mSettingsMenu, mSettingsMainPage);
    else
        LV_LOG_TRACE("something wrong in reset_settings_menu()");
}

void Settings::reset_wifi_menu()
{
    LV_LOG_USER(">>> Settings::reset_wifi_menu()");
    LV_LOG_USER("mWifiOverView=%p", mWifiOverview);
    LV_LOG_USER("mSettingsMainPage = %p", mSettingsMainPage);
    #ifdef OMOTE_ESP32
    if( (uint32_t)mWifiOverview > 0x30000000 ) {
    #else
    if( (uint64_t)mWifiOverview > 0x100000000 ) {
    #endif
        if( lv_obj_get_child_cnt(mWifiOverview) > 0 ) {
            lv_obj_clean(mWifiOverview);
            LV_LOG_USER("clean mWifiOverview");
        } else {
            LV_LOG_USER("mWifiOverview has no childs, nothing to clean");
        }
    }
    else {
        LV_LOG_ERROR("mWifiOverview not initialized %p", mWifiOverview);
        LV_LOG_ERROR("reseting invalid pointer");
        mWifiOverview = nullptr;
    }
    mSSIDLabel = mIPLabel = nullptr;
    lv_obj_t* cont = lv_obj_get_child(mSettingsMainPage, 0);
    create_wifi_main_page(cont);
    LV_LOG_USER("<<< Settings::reset_wifi_menu()");
}

/**
 * @brief IR the settings
 *
 * @param lv_obj_t* parent
 */
void Settings::ir_settings(lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::ir_settings()");
    lv_color_t primary_color = mOmoteUI->getPrimaryColor();
    unsigned int *backlight_brightness = mDisplay->getBacklightBrightness();

    lv_obj_t *menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, "IR");

    lv_obj_t *menuBox = lv_obj_create(parent);
    lv_obj_set_size(menuBox, lv_pct(100), 54);
    lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "IR Receiver");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *irToggle = lv_switch_create(menuBox);
    lv_obj_set_size(irToggle, 40, 22);
    lv_obj_align(irToggle, LV_ALIGN_TOP_RIGHT, 0, -3);
    lv_obj_set_style_bg_color(irToggle, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_add_event_cb(irToggle, IREnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    if (Settings::mHardware->irhandler()->IRReceiver())
        lv_obj_add_state(irToggle, LV_STATE_CHECKED); // set default state
    LV_LOG_USER("<<< Settings::ir_settings()");
}

void Settings::clear_wifi_networks()
{
    lv_obj_clean(this->mWifiSettingsContent);
    this->no_subpages = 0;
}

void Settings::wifi_scan_complete(unsigned int size)
{
    this->no_subpages = (size + WIFI_SUBPAGE_SIZE - 1) / WIFI_SUBPAGE_SIZE;
    this->no_wifi_networks = size;

    if (size == 0)
    {
        lv_obj_t *menuBox = lv_obj_create(this->mWifiSettingsContent);
        lv_obj_set_size(menuBox, lv_pct(100), 45);
        lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);
        lv_obj_t *menuLabel = lv_label_create(menuBox);
        lv_label_set_text(menuLabel, "no networks found");
    }
    else
    {
        this->update_wifi_selection_subpage(0);
    }
}

void Settings::update_wifi_selection_subpage(int page)
{
    #ifdef OMOTE_ESP32
    if (page < this->no_subpages)
    {
        lv_obj_clean(this->mWifiSettingsContent);

        lv_obj_t *pageLabel = lv_label_create(this->mWifiSettingsContent);
        lv_label_set_text_fmt(pageLabel, "Page %d/%d", page + 1, this->no_subpages);
        if (page > 0)
        {
            lv_obj_t *menuBox = lv_obj_create(this->mWifiSettingsContent);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            lv_label_set_text(menuLabel, "Previous");
            lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 0);
            lv_obj_add_event_cb(
                menuBox, [](lv_event_t *e)
                { Settings::getInstance()->next_wifi_selection_subpage(e); },
                LV_EVENT_CLICKED, (void *)(page - 1));
            lv_obj_t *arrow = lv_label_create(menuBox);
            lv_label_set_text(arrow, LV_SYMBOL_LEFT);
            lv_obj_align(arrow, LV_ALIGN_TOP_LEFT, 0, 0);
        }

        for (int i = 0; i < WIFI_SUBPAGE_SIZE && (page * WIFI_SUBPAGE_SIZE + i) < this->no_wifi_networks; i++)
        {
            lv_obj_t *menuBox = lv_obj_create(this->mWifiSettingsContent);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_add_flag(menuBox, LV_OBJ_FLAG_EVENT_BUBBLE);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            //lv_label_set_text(menuLabel, wifihandler.getFoundSSID(page * WIFI_SUBPAGE_SIZE + i).c_str());
            lv_label_set_text(menuLabel,  Settings::mHardware->wifi()->getFoundSSID(page * WIFI_SUBPAGE_SIZE + i).c_str());
            lv_obj_t *wifi_image = lv_img_create(menuBox);
            lv_obj_align(wifi_image, LV_ALIGN_TOP_RIGHT, 0, 0);
            int RSSI = Settings::mHardware->wifi()->getFoundRSSI(page * WIFI_SUBPAGE_SIZE + i);

            if (RSSI > -50)
            {
                lv_img_set_src(wifi_image, &WiFi_High_Signal);
            }
            else if (RSSI > -60)
            {
                lv_img_set_src(wifi_image, &WiFi_Mid_Signal);
            }
            else if (RSSI > -70)
            {
                lv_img_set_src(wifi_image, &WiFi_Low_Signal);
            }
            else
            {
                lv_img_set_src(wifi_image, &WiFi_No_Signal);
            }
            lv_obj_set_style_img_recolor(wifi_image, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_img_recolor_opa(wifi_image, LV_OPA_COVER, LV_PART_MAIN);

            lv_menu_set_load_page_event(mSettingsMenu, menuBox, mWifiPasswordPage);
            lv_obj_add_event_cb(menuBox, wifi_selected_cb, LV_EVENT_CLICKED, mWifiPasswordLabel);
        }

        if ((page + 1) < this->no_subpages)
        {
            lv_obj_t *menuBox = lv_obj_create(this->mWifiSettingsContent);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            lv_label_set_text(menuLabel, "Next");
            lv_obj_add_event_cb(
                menuBox, [](lv_event_t *e)
                { Settings::getInstance()->next_wifi_selection_subpage(e); },
                LV_EVENT_CLICKED, (void *)(page + 1));

            lv_obj_t *arrow = lv_label_create(menuBox);
            lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
            lv_obj_align(arrow, LV_ALIGN_TOP_RIGHT, 0, 0);
        }
        lv_obj_scroll_to_y(this->mWifiSettingsContent, 0, LV_ANIM_OFF);
    }
    #endif
}

void Settings::next_wifi_selection_subpage(lv_event_t *e)
{
    #ifdef OMOTE_ESP32
    int subpage = (int)lv_event_get_user_data(e);
    update_wifi_selection_subpage(subpage);
    #endif
}

void Settings::update_wifi(bool connected)
{
    lv_obj_t *ip_label = mIPLabel; //(mWifiOverview, 3);
    lv_obj_t *ssid_label = this->mSSIDLabel; //(mWifiOverview, 0);
    if (connected)
    {
        LV_LOG_TRACE("update_wifi()");
        LV_LOG_TRACE("mWifiLabel: %p", this->mWifiLabel);
        // lv_label_set_text(this->mWifiLabel, LV_SYMBOL_WIFI);
        mOmoteUI->updateWifi(LV_SYMBOL_WIFI);
        LV_LOG_TRACE("update_wifi() mWifiLabel success");
        LV_LOG_TRACE("ssid_label: %p", ssid_label);
        lv_label_set_text(ssid_label, Settings::mHardware->wifi()->GetStatus().ssid.c_str());
        LV_LOG_TRACE("update_wifi() SSID success");
        LV_LOG_TRACE("ip_label: %p", ip_label);
        //lv_label_set_text(ip_label, wifihandler.getIP().c_str());
        lv_label_set_text(ip_label, Settings::mHardware->wifi()->GetStatus().IP.c_str());
        LV_LOG_TRACE("update_wifi() IP success");
    }
    else
    {
        // lv_label_set_text(this->mWifiLabel, "");
        //this->display->updateWifi(LV_SYMBOL_WIFI);
        mOmoteUI->updateWifi("");
        if( ssid_label )
            lv_label_set_text(ssid_label, "Disconnected");
        if( ip_label )
            lv_label_set_text(ip_label, "-");
    }
}

lv_obj_t *Settings::create_wifi_password_page(lv_obj_t *menu)
{
    LV_LOG_TRACE(">>> Settings::create_wifi_password_page()");

    lv_obj_t *ret_val = lv_menu_page_create(menu, NULL);
    lv_obj_t *cont = lv_menu_cont_create(ret_val);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);

    mWifiPasswordLabel = lv_label_create(cont);
    lv_label_set_text(mWifiPasswordLabel, "Password");
    lv_obj_t *password_input = lv_textarea_create(cont);
    lv_obj_set_width(password_input, lv_pct(100));
    lv_textarea_set_password_mode(password_input, true);
    lv_textarea_set_one_line(password_input, true);
    lv_textarea_set_placeholder_text(password_input, "Password");
    lv_obj_add_event_cb(password_input, ta_event_cb, LV_EVENT_READY, NULL);
    mOmoteUI->attach_keyboard(password_input);

    lv_obj_t *show_password = lv_checkbox_create(cont);
    lv_checkbox_set_text(show_password, "Show password");
    lv_obj_add_event_cb(show_password, show_password_cb, LV_EVENT_VALUE_CHANGED, password_input);

    lv_obj_t *connect_button = lv_btn_create(cont);
    lv_obj_t *label = lv_label_create(connect_button);
    lv_label_set_text(label, "Connect");
    lv_obj_add_event_cb(connect_button, connect_btn_cb, LV_EVENT_CLICKED, password_input);

    LV_LOG_TRACE("<<< Settings::create_wifi_password_page()");

    return ret_val;
}

lv_obj_t *Settings::create_wifi_selection_page(lv_obj_t *menu)
{
    LV_LOG_USER(">>> Settings::create_wifi_selection_page()");

    /* Create sub page for wifi*/
    lv_obj_t *subpage = lv_menu_page_create(menu, NULL);
    mWifiSettingsContent = lv_menu_cont_create(subpage);
    lv_obj_set_layout(mWifiSettingsContent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(mWifiSettingsContent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(mWifiSettingsContent, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *menuLabel = lv_label_create(mWifiSettingsContent);
    lv_label_set_text(menuLabel, "Searching for wifi networks");

    LV_LOG_USER("<<< Settings::create_wifi_selection_page()");

    return subpage;
}

void Settings::create_wifi_main_page(lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::create_wifi_main_page()");
    LV_LOG_USER("mWifiOverview: %p", mWifiOverview);

    lv_color_t primary_color = mOmoteUI->getPrimaryColor();

    if( !mWifiOverview ) {
        mWifiOverview = lv_obj_create(parent);
        lv_obj_set_size(mWifiOverview, lv_pct(100), 90);
        lv_obj_set_style_bg_color(mWifiOverview, primary_color, LV_PART_MAIN);
        lv_obj_set_style_border_width(mWifiOverview, 0, LV_PART_MAIN);
        LV_LOG_USER("create mWifiOverview=%p\n", mWifiOverview);
    }
    else
        LV_LOG_USER("mWifiOverview already existst: %p", mWifiOverview);

    lv_obj_t *enable = lv_label_create(mWifiOverview);
    lv_label_set_text(enable, "Enable WiFi");
    lv_obj_align(enable, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *mWifiEnableSwitch = lv_switch_create(mWifiOverview);
    lv_obj_set_size(mWifiEnableSwitch, 40, 22);
    lv_obj_align(mWifiEnableSwitch, LV_ALIGN_TOP_RIGHT, 0, -3);
    lv_obj_set_style_bg_color(mWifiEnableSwitch, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_add_event_cb(mWifiEnableSwitch, wifiEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, this);
    if( isWifiEnabled() )
    {
        lv_obj_add_state(mWifiEnableSwitch, LV_STATE_CHECKED); // set default state
    
        lv_obj_t* menuLabel = lv_label_create(mWifiOverview);
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 24);
        this->mSSIDLabel = menuLabel;

        lv_obj_t *arrow = lv_label_create(mWifiOverview);
        lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
        lv_obj_align(arrow, LV_ALIGN_TOP_RIGHT, 0, 24);

        lv_obj_t *ip_label = lv_label_create(mWifiOverview);
        lv_label_set_text(ip_label, "IP:");
        lv_obj_align(ip_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

        lv_obj_t *ip = lv_label_create(mWifiOverview);
        lv_obj_align(ip, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        mIPLabel = ip;

        if (Settings::mHardware->wifi()->GetStatus().isConnected)
        {
            lv_label_set_text(mSSIDLabel, Settings::mHardware->wifi()->GetStatus().ssid.c_str());
            lv_label_set_text(mIPLabel, Settings::mHardware->wifi()->GetStatus().IP.c_str());
        }
        else
        {
            lv_label_set_text(menuLabel, "Disconnected");
            lv_label_set_text(ip, "-");
        }
        lv_menu_set_load_page_event(mSettingsMenu, mWifiOverview, mWifiSelectionPage);
        lv_obj_add_event_cb(mWifiOverview, wifi_settings_cb, LV_EVENT_CLICKED, mWifiSettingsContent);
    }

    LV_LOG_USER("mWifiOverview: %p", mWifiOverview);
    LV_LOG_USER("<<< Settings::create_wifi_main_page()");
}

void Settings::create_wifi_settings(lv_obj_t *menu, lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::create_wifi_settings()");

    mWifiSelectionPage = create_wifi_selection_page(menu);
    mWifiPasswordPage = create_wifi_password_page(mSettingsMenu);
    lv_obj_t *menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, "Wi-Fi");
    create_wifi_main_page(parent);

    LV_LOG_USER("<<< Settings::create_wifi_settings()");
}

// bool Settings::addDevice(DeviceInterface *device)
// {
//     LV_LOG_TRACE("Settings::addDevice(%s)", device->getName());

//     DeviceInterface *currentDevice;
//     /* search free slot in device array */
//     for (int i = 0; i < DEVICESLOTS; i++)
//     {
//         if (this->devices[i] == nullptr)
//         {
//             //  Add mTab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
//             this->devices[i] = device;
//             return true;
//         }
//     }
//     return false;
// }

void Settings::saveSettings(){
    LV_LOG_USER("mWifiEnabled: %d", mWifiEnabled);
    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("Settings", false);
    preferences.putBool("mWifiEnabled", mWifiEnabled);
    #endif
    //this->saveAppSettings();
    //this->saveDeviceSettings();
}

void Settings::createDeviceSettings(lv_obj_t *menu, lv_obj_t *parent)
{
    LV_LOG_TRACE("Settings::createDeviceSettings");

    lv_color_t primary_color = mOmoteUI->getPrimaryColor();

    /* search device array */
    // for (int i = 0; i < DEVICESLOTS; i++)
    // {
    //     if (this->devices[i] != nullptr)
    //     {
    //         LV_LOG_TRACE("Seetings::createDeviceSettings Device: %s", this->devices[i]->getName().c_str());
    //         this->devices[i]->displaySettings(parent);
    //     }
    // }
}

void Settings::saveDeviceSettings()
{
    LV_LOG_TRACE("Settings::saveDeviceSettings");

    /* search device array */
    // for (int i = 0; i < DEVICESLOTS; i++)
    // {
    //     if (this->devices[i] != nullptr)
    //     {
    //         LV_LOG_TRACE("Settings::saveDeviceSettings Device: %s", this->devices[i]->getName().c_str());
    //         this->devices[i]->saveSettings();
    //     }
    // }
}

void Settings::createAppSettings(lv_obj_t *menu, lv_obj_t *parent)
{
    LV_LOG_USER(">>> Settings::createAppSettings\n");

    lv_color_t primary_color = mOmoteUI->getPrimaryColor();

    /* search app array */
    for (int i = 0; i < APPSLOTS; i++)
    {
        AppInterface* app = mOmoteUI->getApp(i);
        if (app != nullptr && app->getName() != "Settings" )
        {
            LV_LOG_USER("Settings::createAppSettings App: %s\n", app->getName().c_str());
            app->displaySettings(parent);
        }
    }
    LV_LOG_USER("<<< Settings::createAppSettings\n");
}

void Settings::saveAppSettings()
{
    LV_LOG_TRACE(">>> Settings::saveAppSettings");

    /* search app array */
    // for (int i = 0; i < APPSLOTS; i++)
    // {
    //     if (this->apps[i] != nullptr)
    //     {
    //         LV_LOG_TRACE("Settings::saveAppSettings app: %s", this->apps[i]->getName().c_str());
    //         this->apps[i]->saveSettings();
    //     }
    // }
    LV_LOG_TRACE("<<< Settings::saveAppSettings");
}
