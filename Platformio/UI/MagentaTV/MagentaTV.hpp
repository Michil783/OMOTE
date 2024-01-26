#include <Display.hpp>
#include <DeviceInterface.hpp>
#include <Settings.hpp>
#include <MR401.hpp>
#include <SamsungTV.hpp>

#ifndef _MAGENTATV_HPP_
#define _MAGENTATV_HPP_

class MagentaTV : public AppInterface {
    public:
    MagentaTV(Display* display);
    void setup();
    void handleCustomKeypad(int keyCode, char keyChar);
    String getName(){return "MagentaTV";};
    void displaySettings(lv_obj_t *parent);
    void saveSettings();
    void resetMagentaTVPage();

    lv_obj_t* cPicker = nullptr;
    
    lv_color_t bgColor;
    lv_color_t textColor;
    lv_obj_t* colorPickerPage = nullptr;

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
     * @brief setup MagentaTV main page
     *
     * @param lv_obj_t* parent
     */
    void setup_MagentaTV(lv_obj_t *parent);

    MR401* mr401;
    SamsungTV* samsung;

    /**
     * @brief LVGL Menu for settings pages as needed.
     *
     */
    lv_obj_t *settingsMenu = nullptr;
    lv_obj_t *menuLabel = nullptr;
    lv_obj_t* menuBox = nullptr;
    bool iconEnabled = false;
};

#endif