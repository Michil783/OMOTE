#include <Settings.hpp>
#include <Display.hpp>
#include <Arduino.h>
#include <WifiHandler.hpp>
#include <IRHandler.hpp>

#define WIFI_SUBPAGE_SIZE 3

LV_IMG_DECLARE(high_brightness);
LV_IMG_DECLARE(low_brightness);
LV_IMG_DECLARE(WiFi_No_Signal);
LV_IMG_DECLARE(WiFi_Low_Signal);
LV_IMG_DECLARE(WiFi_Mid_Signal);
LV_IMG_DECLARE(WiFi_High_Signal);

extern bool wakeupByIMUEnabled;
extern Display display;
extern Settings settings;
extern long standbyTimerConfigured;
extern WifiHandler wifihandler;
static char *ssid;
extern IRHandler irhandler;

/**
 * @brief Textarea callback function for the password field. In case the enter key is pressed in the text area, the
 * function will try to connect to the network with the provided password.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
static void ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);

    const char *password = lv_textarea_get_text(ta);
    switch (code)
    {
    case LV_EVENT_READY:
        wifihandler.connect(ssid, password);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        display.hide_keyboard();
        settings.reset_settings_menu();
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
void WakeEnableSetting_event_cb(lv_event_t *e)
{
    // LV_LOG_TRACE("Settings - WakeEnableSetting_event_cb");
    wakeupByIMUEnabled = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
}

/**
 * @brief Checkbox callback function for the IR receiver option
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void IREnableSetting_event_cb(lv_event_t *e)
{
    LV_LOG_TRACE("Settings - IREnableSetting_event_cb");
    irhandler.IRReceiverEnable(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
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
    *backlight_brightness = map(constrain(lv_slider_get_value(slider), 30, 240), 30, 240, 240, 30);
    // LV_LOG_TRACE("Settings - bl_slider_event_cb(%d) - %d", constrain(lv_slider_get_value(slider), 30, 240), *backlight_brightness);
}

/**
 * @brief Dropdown list callback function for the screen timeout duration.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void to_dropdown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int index = lv_dropdown_get_selected(obj);
        // LV_LOG_TRACE("new value = %d\n", index);
        long *standbyTimerConfigured = (long *)lv_event_get_user_data(e);
        switch (index)
        {
        case 1:
            *standbyTimerConfigured = 30000;
            break;
        case 2:
            *standbyTimerConfigured = 60000;
            break;
        case 3:
            *standbyTimerConfigured = 500000;
            break;
        default:
            *standbyTimerConfigured = 10000;
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
static void wifi_selected_cb(lv_event_t *e)
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
static void wifi_settings_cb(lv_event_t *event)
{
    lv_obj_t *cont = (lv_obj_t *)lv_event_get_user_data(event);
    lv_obj_clean(cont);
    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, "Searching for wifi networks");
    // This will trigger an asynchronouse network scan
    wifihandler.scan();
}

/**
 * @brief Callback which is triggered when clicking the connect button. It triggers the wifi connection.
 *
 * @param event Pointer to event object for the event where this callback is called
 */
static void connect_btn_cb(lv_event_t *event)
{
    lv_obj_t *ta = (lv_obj_t *)event->user_data;
    const char *password = lv_textarea_get_text(ta);

    wifihandler.connect(ssid, password);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    display.hide_keyboard();
    settings.reset_settings_menu();
}

/**
 * @brief Callback function for the show password checkbox. Checking the box will show the password while unchecked the
 * password will be shown as dots.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
static void show_password_cb(lv_event_t *e)
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
static lv_obj_t *kb;
static lv_obj_t *ta;

/**
 * @brief Setrtings constructor
 *
 * @param Display* display
 */

Settings::Settings(Display *display)
{
    this->display = display;

    /*Initialize device array*/
    for (int i = 0; i < DEVICESLOTS; i++)
    {
        this->devices[i] = nullptr;
    }
}

String Settings::getName(){
    return "Settings";
}

/**
 * @brief setup of the settings screen
 * function will build display and Wifi setings areas
 *
 * @param
 */
void Settings::setup()
{
    this->tab = this->display->addTab(this);

    /* Create main page for settings this->settingsMenu*/
    this->setup_settings(this->tab);
}

/**
 * @brief build the settings in LVGL
 *
 * @param lv_obj_t* parent
 */
void Settings::setup_settings(lv_obj_t *parent)
{
    // Add content to the settings tab
    // With a flex layout, setting groups/boxes will position themselves automatically
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);
    // Add a label, then a box for the display settings
    this->settingsMenu = lv_menu_create(parent);
    lv_obj_set_width(this->settingsMenu, 210);

    /* Create main page for settings this->settingsMenu*/
    this->settingsMainPage = lv_menu_page_create(this->settingsMenu, NULL);
    lv_obj_t *cont = lv_menu_cont_create(this->settingsMainPage);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);

    this->display_settings(cont);

    this->ir_settings(cont);

    this->create_wifi_settings(this->settingsMenu, cont);

    this->createAppSettings(this->settingsMenu, cont);

    this->createDeviceSettings(this->settingsMenu, cont);

    lv_menu_set_page(this->settingsMenu, this->settingsMainPage);
}

/**
 * @brief display the settings
 *
 * @param lv_obj_t* parent
 */
void Settings::display_settings(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

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
    lv_slider_set_value(slider, map(*backlight_brightness, 240, 30, 30, 240), LV_ANIM_OFF);
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
    lv_obj_add_event_cb(wakeToggle, WakeEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    if (wakeupByIMUEnabled)
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
    switch (standbyTimerConfigured)
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
    lv_obj_add_event_cb(drop, to_dropdown_event_cb, LV_EVENT_ALL, &standbyTimerConfigured);
}

void Settings::reset_settings_menu()
{
    /*
    lv_obj_clean(this->tab);
    this->setup_settings(this->tab);
    */
    if( this->settingsMenu && this->settingsMainPage )
        lv_menu_set_page(this->settingsMenu, this->settingsMainPage);
    else
        LV_LOG_TRACE("something wrong in reset_settings_menu()");
}


/**
 * @brief IR the settings
 *
 * @param lv_obj_t* parent
 */
void Settings::ir_settings(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

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
    if (irhandler.IRReceiver())
        lv_obj_add_state(irToggle, LV_STATE_CHECKED); // set default state
}

void Settings::clear_wifi_networks()
{
    lv_obj_clean(this->wifi_setting_cont);
    this->no_subpages = 0;
}

void Settings::wifi_scan_complete(unsigned int size)
{
    this->no_subpages = (size + WIFI_SUBPAGE_SIZE - 1) / WIFI_SUBPAGE_SIZE;
    this->no_wifi_networks = size;

    if (size == 0)
    {
        lv_obj_t *menuBox = lv_obj_create(this->wifi_setting_cont);
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
    if (page < this->no_subpages)
    {
        lv_obj_clean(this->wifi_setting_cont);

        lv_obj_t *pageLabel = lv_label_create(this->wifi_setting_cont);
        lv_label_set_text_fmt(pageLabel, "Page %d/%d", page + 1, this->no_subpages);
        if (page > 0)
        {
            lv_obj_t *menuBox = lv_obj_create(this->wifi_setting_cont);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            lv_label_set_text(menuLabel, "Previous");
            lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 0);
            lv_obj_add_event_cb(
                menuBox, [](lv_event_t *e)
                { settings.next_wifi_selection_subpage(e); },
                LV_EVENT_CLICKED, (void *)(page - 1));
            lv_obj_t *arrow = lv_label_create(menuBox);
            lv_label_set_text(arrow, LV_SYMBOL_LEFT);
            lv_obj_align(arrow, LV_ALIGN_TOP_LEFT, 0, 0);
        }

        for (int i = 0; i < WIFI_SUBPAGE_SIZE && (page * WIFI_SUBPAGE_SIZE + i) < this->no_wifi_networks; i++)
        {
            lv_obj_t *menuBox = lv_obj_create(this->wifi_setting_cont);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_add_flag(menuBox, LV_OBJ_FLAG_EVENT_BUBBLE);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            lv_label_set_text(menuLabel, wifihandler.getFoundSSID(page * WIFI_SUBPAGE_SIZE + i).c_str());
            lv_obj_t *wifi_image = lv_img_create(menuBox);
            lv_obj_align(wifi_image, LV_ALIGN_TOP_RIGHT, 0, 0);
            int RSSI = wifihandler.getFoundRSSI(page * WIFI_SUBPAGE_SIZE + i);

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

            lv_menu_set_load_page_event(this->settingsMenu, menuBox, this->wifi_password_page);
            lv_obj_add_event_cb(menuBox, wifi_selected_cb, LV_EVENT_CLICKED, this->wifi_password_label);
        }

        if ((page + 1) < this->no_subpages)
        {
            lv_obj_t *menuBox = lv_obj_create(this->wifi_setting_cont);
            lv_obj_set_size(menuBox, lv_pct(100), 45);
            lv_obj_set_scrollbar_mode(menuBox, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t *menuLabel = lv_label_create(menuBox);
            lv_label_set_text(menuLabel, "Next");
            lv_obj_add_event_cb(
                menuBox, [](lv_event_t *e)
                { settings.next_wifi_selection_subpage(e); },
                LV_EVENT_CLICKED, (void *)(page + 1));

            lv_obj_t *arrow = lv_label_create(menuBox);
            lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
            lv_obj_align(arrow, LV_ALIGN_TOP_RIGHT, 0, 0);
        }
        lv_obj_scroll_to_y(this->wifi_setting_cont, 0, LV_ANIM_OFF);
    }
}

void Settings::next_wifi_selection_subpage(lv_event_t *e)
{
    int subpage = (int)lv_event_get_user_data(e);
    this->update_wifi_selection_subpage(subpage);
}

void Settings::update_wifi(bool connected)
{

    lv_obj_t *ip_label = lv_obj_get_child(this->wifiOverview, 3);
    lv_obj_t *ssid_label = lv_obj_get_child(this->wifiOverview, 0);
    if (connected)
    {
        LV_LOG_TRACE("update_wifi()");
        LV_LOG_TRACE("WifiLabel: %p", this->WifiLabel);
        // lv_label_set_text(this->WifiLabel, LV_SYMBOL_WIFI);
        this->display->updateWifi(LV_SYMBOL_WIFI);
        LV_LOG_TRACE("update_wifi() WifiLabel success");
        LV_LOG_TRACE("ssid_label: %p", ssid_label);
        lv_label_set_text(ssid_label, wifihandler.getSSID());
        LV_LOG_TRACE("update_wifi() SSID success");
        LV_LOG_TRACE("ip_label: %p", ip_label);
        lv_label_set_text(ip_label, wifihandler.getIP().c_str());
        LV_LOG_TRACE("update_wifi() IP success");
    }
    else
    {
        // lv_label_set_text(this->WifiLabel, "");
        this->display->updateWifi(LV_SYMBOL_WIFI);
        lv_label_set_text(ssid_label, "Disconnected");
        lv_label_set_text(ip_label, "-");
    }
}

lv_obj_t *Settings::create_wifi_password_page(lv_obj_t *menu)
{
    LV_LOG_TRACE("Settings::create_wifi_password_page()");

    lv_obj_t *ret_val = lv_menu_page_create(menu, NULL);
    lv_obj_t *cont = lv_menu_cont_create(ret_val);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);

    this->wifi_password_label = lv_label_create(cont);
    lv_label_set_text(this->wifi_password_label, "Password");
    lv_obj_t *password_input = lv_textarea_create(cont);
    lv_obj_set_width(password_input, lv_pct(100));
    lv_textarea_set_password_mode(password_input, true);
    lv_textarea_set_one_line(password_input, true);
    lv_textarea_set_placeholder_text(password_input, "Password");
    lv_obj_add_event_cb(password_input, ta_event_cb, LV_EVENT_READY, NULL);
    this->display->attach_keyboard(password_input);

    lv_obj_t *show_password = lv_checkbox_create(cont);
    lv_checkbox_set_text(show_password, "Show password");
    lv_obj_add_event_cb(show_password, show_password_cb, LV_EVENT_VALUE_CHANGED, password_input);

    lv_obj_t *connect_button = lv_btn_create(cont);
    lv_obj_t *label = lv_label_create(connect_button);
    lv_label_set_text(label, "Connect");
    lv_obj_add_event_cb(connect_button, connect_btn_cb, LV_EVENT_CLICKED, password_input);

    return ret_val;
}

lv_obj_t *Settings::create_wifi_selection_page(lv_obj_t *menu)
{
    /* Create sub page for wifi*/
    lv_obj_t *subpage = lv_menu_page_create(menu, NULL);
    this->wifi_setting_cont = lv_menu_cont_create(subpage);
    lv_obj_set_layout(this->wifi_setting_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(this->wifi_setting_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(this->wifi_setting_cont, LV_SCROLLBAR_MODE_ACTIVE);

    lv_obj_t *menuLabel = lv_label_create(this->wifi_setting_cont);
    lv_label_set_text(menuLabel, "Searching for wifi networks");

    return subpage;
}

void Settings::create_wifi_main_page(lv_obj_t *parent)
{
    lv_color_t primary_color = display->getPrimaryColor();

    lv_obj_t *menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, "Wi-Fi");
    this->wifiOverview = lv_obj_create(parent);
    lv_obj_set_size(this->wifiOverview, lv_pct(100), 80);
    lv_obj_set_style_bg_color(this->wifiOverview, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(this->wifiOverview, 0, LV_PART_MAIN);
    menuLabel = lv_label_create(this->wifiOverview);

    lv_obj_t *arrow = lv_label_create(this->wifiOverview);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
    lv_obj_align(arrow, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *ip_label = lv_label_create(this->wifiOverview);
    lv_label_set_text(ip_label, "IP:");
    lv_obj_align(ip_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *ip = lv_label_create(this->wifiOverview);
    lv_obj_align(ip, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    if (wifihandler.isConnected())
    {
        lv_label_set_text(menuLabel, wifihandler.getSSID());
        lv_label_set_text(ip, wifihandler.getIP().c_str());
    }
    else
    {
        lv_label_set_text(menuLabel, "Disconnected");
        lv_label_set_text(ip, "-");
    }
    lv_menu_set_load_page_event(this->settingsMenu, this->wifiOverview, this->wifi_selection_page);
    lv_obj_add_event_cb(this->wifiOverview, wifi_settings_cb, LV_EVENT_CLICKED, this->wifi_setting_cont);
}

void Settings::create_wifi_settings(lv_obj_t *menu, lv_obj_t *parent)
{
    this->wifi_selection_page = this->create_wifi_selection_page(menu);
    this->wifi_password_page = this->create_wifi_password_page(this->settingsMenu);
    this->create_wifi_main_page(parent);
}

bool Settings::addDevice(DeviceInterface *device)
{
    LV_LOG_TRACE("Settings::addDevice(%s)", device->getName());

    DeviceInterface *currentDevice;
    /* search free slot in device array */
    for (int i = 0; i < DEVICESLOTS; i++)
    {
        if (this->devices[i] == nullptr)
        {
            //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
            this->devices[i] = device;
            return true;
        }
    }
    return false;
}

void Settings::saveSettings(){
    this->saveAppSettings();
    this->saveDeviceSettings();
}

void Settings::createDeviceSettings(lv_obj_t *menu, lv_obj_t *parent)
{
    LV_LOG_TRACE("Settings::createDeviceSettings");

    lv_color_t primary_color = display->getPrimaryColor();

    /* search device array */
    for (int i = 0; i < DEVICESLOTS; i++)
    {
        if (this->devices[i] != nullptr)
        {
            LV_LOG_TRACE("Seetings::createDeviceSettings Device: %s", this->devices[i]->getName().c_str());
            this->devices[i]->displaySettings(parent);
        }
    }
}

void Settings::saveDeviceSettings()
{
    LV_LOG_TRACE("Settings::saveDeviceSettings");

    /* search device array */
    for (int i = 0; i < DEVICESLOTS; i++)
    {
        if (this->devices[i] != nullptr)
        {
            LV_LOG_TRACE("Settings::saveDeviceSettings Device: %s", this->devices[i]->getName().c_str());
            this->devices[i]->saveSettings();
        }
    }
}

bool Settings::addApp(AppInterface *app)
{
    LV_LOG_TRACE("Settings::addApp(%s)", app->getName());

    AppInterface *currentApp;
    /* search free slot in app array */
    for (int i = 0; i < APPSLOTS; i++)
    {
        if (this->apps[i] == nullptr)
        {
            //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
            this->apps[i] = app;
            return true;
        }
    }
    return false;
}

void Settings::createAppSettings(lv_obj_t *menu, lv_obj_t *parent)
{
    LV_LOG_TRACE("Settings::createAppSettings");

    lv_color_t primary_color = display->getPrimaryColor();

    /* search app array */
    for (int i = 0; i < APPSLOTS; i++)
    {
        if (this->apps[i] != nullptr)
        {
            LV_LOG_TRACE("Settings::createAppSettings App: %s", this->apps[i]->getName().c_str());
            this->apps[i]->displaySettings(parent);
        }
    }
}

void Settings::saveAppSettings()
{
    LV_LOG_TRACE("Settings::saveAppSettings");

    /* search app array */
    for (int i = 0; i < APPSLOTS; i++)
    {
        if (this->apps[i] != nullptr)
        {
            LV_LOG_TRACE("Settings::saveAppSettings app: %s", this->apps[i]->getName().c_str());
            this->apps[i]->saveSettings();
        }
    }
}
