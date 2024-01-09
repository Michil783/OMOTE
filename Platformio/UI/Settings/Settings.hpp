#include <Display.hpp>
#include <WiFi.h>

#ifndef _SETTINGS_HPP_
#define _SETTINGS_HPP_

class Settings
{
public:
    Settings(Display *display);
    void setup();

    /**
     * @brief Get the Percentage of the battery
     *
     * @return int Percentage of the battery
     */

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

private:
    /**
     * @brief pointer to Display object
     *
     */
    Display *display;

    /**
     * @brief tab object
     *
     */
    lv_obj_t *tab;

    /**
     * @brief LVGL Menu for settings pages as needed.
     *
     */
    lv_obj_t *settingsMenu;

    /**
     * @brief Main page of the settings menu
     *
     */
    lv_obj_t *settingsMainPage;

    /**
     * @brief setup settings main page
     *
     * @param lv_obj_t* parent
     */
    void setup_settings(lv_obj_t *parent);

    /**
     * @brief Function to create the display settings page.
     *
     * @param parent LVGL object acting as a parent for the display settings page
     */
    void display_settings(lv_obj_t *parent);

    /**
     * @brief Container within the wifi selection page
     */
    lv_obj_t *wifi_setting_cont;

    /**
     * @brief Wifi settings entry point on the settings tab
     *
     */
    lv_obj_t *wifiOverview;

    /**
     * @brief Label in the wifi password page. This label is updated with the selected SSID when the credentials for
     * a wifi network is entered.
     *
     */
    lv_obj_t *wifi_password_label;

    /**
     * @brief Menu Subpage for the wifi password
     */
    lv_obj_t *wifi_password_page;

    /**
     * @brief  Menu Subpage for wifi selection
     */
    lv_obj_t *wifi_selection_page;

    /**
     * @brief Wifi Label shown in the top status bar
     */
    lv_obj_t *WifiLabel;

    /**
     * @brief Number of wifi subpage needed to display the found wifi networks
     *
     */
    unsigned int no_subpages;

    /**
     * @brief number of wifi networks found
     *
     */
    unsigned int no_wifi_networks;

    /**
     * @brief callback function to get next wifi subpage. This callback can be used to get the next or previous page
     *
     * @param e lvgl event object
     */
    void next_wifi_selection_subpage(lv_event_t *e);

    /**
     * @brief Create a wifi selection sub page object
     *
     * @param menu         LVGL Menu where the sub page should be added to
     * @return lv_obj_t*   Menu sub page object pointer
     */
    lv_obj_t *create_wifi_selection_page(lv_obj_t *menu);

    /**
     * @brief Method to create the wifi password sub page
     *
     * @param menu        Menu where the sub page should be created
     * @return lv_obj_t*  menu sub page object pointer
     */
    lv_obj_t *create_wifi_password_page(lv_obj_t *menu);

    /**
     * @brief Method to create the wifi settings on the main page
     *
     * @param parent    lv object parent where the main settings page should be added to
     */
    void create_wifi_main_page(lv_obj_t *parent);

    /**
     * @brief Method to create wifi settings. This method will call the create_wifi_selection_page,
     * the create_wifi_password_page, and the create_wifi_main_page
     *
     * @param menu      Settings menu where the sub pages should be added to
     * @param parent    lv object parent where the main settings page should be added to
     */
    void create_wifi_settings(lv_obj_t *menu, lv_obj_t *parent);

    /**
     * @brief Function to update the wifi selection sub page
     *
     * @param page index of the page to display
     */
    void update_wifi_selection_subpage(int page);
};

#endif