/**
 * @file display.hpp
 * @author Thomas Bittner
 * @brief Header file describing the display class used to create the lvgl object to handle display input and outpu
 * @version 0.1
 * @date 2023-08-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#include <lvgl.h>
#include <omote.hpp>
#include <AppInterface.hpp>
//#include <string>

#define DEFAULT_BACKLIGHT_BRIGHTNESS 128
#define TAB_ARRAY_SIZE 10

class Display
{
    public:
        Display(int backlight_pin, int enable_pin, int width, int height);

        /**
         * @brief  Function to setup the display.
         * 
         */
        void setup();

        /**
         * @brief Function to setup the user interface. This function has to be called after setup.
         * 
         */
        void setup_ui();

        /**
         * @brief Function to flush the display (update what is shown on the LCD). This function is called within
         * the LVGL callback function 
         * 
         * @param disp 
         * @param area 
         * @param color_p 
         */
        void flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );

        /**
         * @brief Function to hide the keyboard. If the keyboard is attached to a text area, it will be hidden when the
         * text area is defocused. This function can be used if the keyboard need to be hidden due to some script event. 
         * 
         */
        void hide_keyboard();

        /**
         * @brief Function to show the keyboard. If a text area needs the keybaord, it should be attached to the text area
         * using the approbiate function. The keyboard will then show up when the text area is focused. This function is
         * needed if the keyboard should be shown due to some script or other trigger.
         * 
         */
        void show_keyboard();

        /**
         *  @brief API function which needs to be called regularly to update the display
         * 
         */
        void update();

        /**
         * @brief Function to change the settings menu to main page again.
         * 
         */
        void reset_settings_menu();

        /**
         * @brief Function to update the battery indicator on the display 
         * 
         * @param percentage Battery percentage 
         * @param isCharging  True if the battery is charging. False otherwise
         * @param isConnected True if a battery is connected, false otherwise
         */
        void update_battery(int percentage, bool isCharging, bool isConnected);

        //void updateWifi(std::string symbol);
        void updateWifi(const char* symbol);

        void turnOff();

        lv_obj_t* addTab(AppInterface* app);

        lv_obj_t* getTabView();

        void setActiveTab(unsigned char tab);

        AppInterface* getApp(unsigned char tab);

        /**
         * @brief Get the primary_color
         * 
         * @param 
         */
        lv_color_t getPrimaryColor();

        unsigned int* getBacklightBrightness();

        /**
         * @brief Function to attach the keyboard to a text area. If the text area is selected, the keyboard
         * is shown and if the textarea is defocused, the keyboard will be hidden again.
         * 
         * @param textarea Textarea where the keyboard should be attached to. 
         */
        void attach_keyboard(lv_obj_t* textarea);

    private:
        /**
         * @brief Pin used for LCD backlight control
         * 
         */
        int backlight_pin;

        /**
         * @brief Pin used to enable the LCD 
         * 
         */
        int enable_pin;

        /**
         * @brief Width of the display in pixel 
         * 
         */
        int width;

        /**
         * @brief Height of the display in pixel 
         * 
         */
        int height;

        /**
         * @brief Pointer to the buffer used for drawing on the screen 
         * 
         */
        lv_color_t *bufA;

        /**
         * @brief Pointer to the buffer used for drawing on the screen 
         * 
         */
        lv_color_t *bufB;
        
        /**
         * @brief Keyboard object used whenever a keyboard is needed.
         * 
         */
        lv_obj_t* kb;

        /**
         * @brief Variable to store the primary color
         * 
         */
        lv_color_t primary_color;

        /**
         * @brief Set the up settings object
         * 
         * @param parent 
         */
        void setup_settings(lv_obj_t* parent);

        /**
         * @brief Function to create the keyboard object which can then be attached to different text areas.
         * 
         */
        void create_keyboard();

        /**
         * @brief LVGL Menu for settings pages as needed.
         * 
         */
        lv_obj_t* settingsMenu;

        /**
         * @brief Main page of the settings menu 
         * 
         */
        //lv_obj_t* resetsettingsMainPage;

        /**
         * @brief Battery percentage label 
         * 
         */
        lv_obj_t* objBattPercentage;

        /**
         * @brief Battery icon object in the status bar 
         * 
         */
        lv_obj_t* objBattIcon;

        /**
         * @brief USB icon object in the status bar 
         * 
         */
        lv_obj_t* objUSBIcon;

        lv_obj_t* tabview;
        
        /**
         * @brief Array of tab names
         * 
        */
        //const char *tabNames[TAB_ARRAY_SIZE];
        AppInterface* apps[TAB_ARRAY_SIZE];

        /**
         * @brief Function to create the tab view buttons
         * 
         */
        void createTabviewButtons();

/******************************************** Statusbar *************************************************************/
        /**
         * Variable to store the Statusbar container
        */
        lv_obj_t* statusbar;

        /**
         * @brief Wifi Label shown in the top status bar
         */
        lv_obj_t* WifiLabel;

        /**
         * Variable to store the current backlight brightness level
        */
        unsigned int backlight_brightness;

        #define BATTERYCHARGINGINDEX_MAX 5
        int batteryChargingIndex = 0;
        const char* batteryCharging[BATTERYCHARGINGINDEX_MAX] = {LV_SYMBOL_BATTERY_EMPTY, LV_SYMBOL_BATTERY_1, LV_SYMBOL_BATTERY_2, LV_SYMBOL_BATTERY_3, LV_SYMBOL_BATTERY_FULL};
};



#endif