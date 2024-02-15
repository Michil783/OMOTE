#pragma once
#include "OmoteUI.hpp"
#include "HardwareAbstract.hpp"
#include "DisplayAbstract.h"
#include <DeviceInterface.hpp>
#include <AppInterface.hpp>
#include <MR401.hpp>
#include <SamsungTV.hpp>
#include <string>

class MagentaTV : public AppInterface {
    public:
    static std::shared_ptr<MagentaTV> getInstance() {return std::shared_ptr<MagentaTV>(MagentaTV::mInstance);}
    MagentaTV(std::shared_ptr<DisplayAbstract> display);

    /* AppInterface */
    //void addAppSettings(std::shared_ptr<void> settings) override;
    void handleCustomKeypad(int keyCode, char keyChar) override;
    std::string getName() override { return "MagentaTV"; };
    void displaySettings(lv_obj_t *parent) override;
    void saveSettings() override;

    void setup();
    void resetMagentaTVPage();
    void setFontSize(int newSize) { fontSize = newSize; };
    int getFontSize() { return fontSize; };

    lv_color_t bgColor;
    lv_color_t textColor;

    private:
    static MagentaTV* mInstance;
    static std::shared_ptr<UI::Basic::OmoteUI> mOmoteUI;

    std::shared_ptr<DisplayAbstract> display;
    lv_obj_t *tab;
    void setup_MagentaTV(lv_obj_t *parent);

    static std::shared_ptr<MR401> mMR401;
    static std::shared_ptr<SamsungTV> mSamsung;

    static void virtualKeypad_event_cb(lv_event_t *e);
    static void fs_dropdown_event_cb(lv_event_t *e);
    static void IconEnableSetting_event_cb(lv_event_t *e);
    static void selectColor_event_cb(lv_event_t* e);
    static void selectColor_ok_event_cb(lv_event_t* e);
    static void selectColor_close_event_cb(lv_event_t *e);
    static void colorPicker_event_cb(lv_event_t *e);

    lv_obj_t *mSettingsMenu = nullptr;
    lv_obj_t *mMenuLabel = nullptr;
    lv_obj_t* mMenuBox = nullptr;
    bool mIconEnabled = false;

    lv_obj_t* cPicker = nullptr;
    
    lv_obj_t* colorPickerPage = nullptr;

    int fontSize = 1;
    lv_font_t usedFont[3] = {
        lv_font_montserrat_18,
        lv_font_montserrat_20,
        lv_font_montserrat_22
    };
};
