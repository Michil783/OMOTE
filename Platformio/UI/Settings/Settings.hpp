#include <Display.hpp>

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
     * @brief Function to update the battery status. This should be called on a regular basis
     *
     */
    void update();

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
};

#endif