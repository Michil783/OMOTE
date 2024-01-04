#include <Statusbar.hpp>

//#define DEBUG
//#define BATTERY_PERCENTAGE

Statusbar::Statusbar(lv_obj_t * activeScreen) {
    this->activeScreen = activeScreen;
    this->statusbar = lv_btn_create(lv_scr_act());
    lv_obj_set_size(this->statusbar, 240, 20);
    lv_obj_set_style_shadow_width(this->statusbar, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(this->statusbar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_radius(this->statusbar, 0, LV_PART_MAIN);
    lv_obj_align(this->statusbar, LV_ALIGN_TOP_MID, 0, 0);

    this->objWifiLabel = lv_label_create(this->statusbar);
    lv_label_set_text(this->objWifiLabel, "");
    lv_obj_align(this->objWifiLabel, LV_ALIGN_LEFT_MID, -8, 0);
    lv_obj_set_style_text_font(this->objWifiLabel, &lv_font_montserrat_12, LV_PART_MAIN);

    this->objUSBIcon = lv_label_create(this->statusbar);
    lv_label_set_text(this->objUSBIcon, "");
    lv_obj_align(this->objUSBIcon, LV_ALIGN_RIGHT_MID, -24, 0);
    lv_obj_set_style_text_font(this->objUSBIcon, &lv_font_montserrat_12, LV_PART_MAIN);

    this->objBattPercentage = lv_label_create(this->statusbar);
    lv_label_set_text(this->objBattPercentage, "");
    lv_obj_align(this->objBattPercentage, LV_ALIGN_RIGHT_MID, 8, 0);
    lv_obj_set_style_text_font(this->objBattPercentage, &lv_font_montserrat_8, LV_PART_MAIN);

    this->objBattIcon = lv_label_create(this->statusbar);
    lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_align(this->objBattIcon, LV_ALIGN_RIGHT_MID, 8, 0);
    lv_obj_set_style_text_font(this->objBattIcon, &lv_font_montserrat_16, LV_PART_MAIN);

    this->batteryTaskTimer = millis() + 1000;
}

void Statusbar::loop() {
    if(millis() - this->batteryTaskTimer >= 1000){
        this->battery_voltage = analogRead(ADC_BAT)*2*3300/4095 + 350; // 350mV ADC offset
        this->battery_percentage = constrain(map(battery_voltage, 3700, 4200, 0, 100), 0, 100);
        this->batteryTaskTimer = millis();
        this->battery_ischarging = !digitalRead(CRG_STAT);
        Serial.printf("isCharging %d voltage %d\n", this->battery_ischarging, this->battery_voltage);
        // Check if battery is charging, fully charged or disconnected
        if(this->battery_voltage > 4350){
            lv_label_set_text(this->objUSBIcon, LV_SYMBOL_USB);
        }
        #ifdef DEBUG
        char voltage[4];
        sprintf(voltage, "%d%%", this->battery_percentage);
        lv_label_set_text(this->objBattPercentage, voltage);
        #endif
        if(this->battery_ischarging || (!this->battery_ischarging && this->battery_voltage > 4350)){
            #ifdef BATTERY_PERCENTAGE
            lv_label_set_text(this->objBattPercentage, "");
            #endif
            lv_label_set_text(this->objBattIcon, LV_SYMBOL_CHARGE);
        }
        else{
            // Update status bar battery indicator
            #ifdef BATTERY_PERCENTAGE
            lv_label_set_text_fmt(objBattPercentage, "%d%%", battery_percentage);
            #endif
            if(this->battery_percentage > 95) lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_FULL);
            else if(this->battery_percentage > 75) lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_3);
            else if(this->battery_percentage > 50) lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_2);
            else if(this->battery_percentage > 25) lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_1);
            else lv_label_set_text(this->objBattIcon, LV_SYMBOL_BATTERY_EMPTY);
        }
    }
}

void Statusbar::WifiLabel(String label) {
    lv_label_set_text(this->objWifiLabel, label.c_str());
}