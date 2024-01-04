#include <Arduino.h>
#include <lvgl.h>
#include <omote.hpp>

class Statusbar {
public:
    Statusbar(lv_obj_t *);
    void loop();
    void WifiLabel(String label);
    
private:
    unsigned long batteryTaskTimer;
    lv_obj_t* activeScreen;
    lv_obj_t* statusbar;
    lv_obj_t* objWifiLabel;
    lv_obj_t* objBattPercentage;
    lv_obj_t* objBattIcon;
    lv_obj_t* objUSBIcon;
    int battery_voltage;
    int battery_percentage;
    bool battery_ischarging;
};