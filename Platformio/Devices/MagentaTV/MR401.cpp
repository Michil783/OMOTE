#include <Arduino.h>
#include <MR401.hpp>
#include <Display.hpp>
#include <Preferences.h>

#include <omote.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;

/**
 * @brief Dropdown list callback function for the screen timeout duration.
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void mr401_dropdown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        int index = lv_dropdown_get_selected(obj);
        // Serial.printf("new value = %d\n", index);
        int *irCodeTable = (int *)lv_event_get_user_data(e);
        // Serial.printf("begin mr401_dropdown_event_cb irCodeTable: %d\n", *irCodeTable);
        switch (index)
        {
        case 1:
            *irCodeTable = 1;
            break;
        case 2:
            *irCodeTable = 2;
            break;
        case 3:
            *irCodeTable = 3;
            break;
        default:
            *irCodeTable = 0;
            break;
        }
        // Serial.printf("end mr401_dropdown_event_cb irCodeTable: %d\n", *irCodeTable);
    }
}

MR401::MR401(Display *display)
{
    this->display = display;
}

void MR401::saveSettings()
{
    // Serial.printf("MR401 saveSettings codeTable: %d\n", this->irCodeTable);
    preferences.putInt("MR401CodeTable", this->irCodeTable);
}

String MR401::getName()
{
    return "MR401";
}

void MR401::handleCustomKeypad(int keyCode){
    Serial.println("MR401");
    Serial.printf("handleCustomKeypad(%d)\n", keyCode);
    if( keyCode == 16 ) // Power
        IrSender.sendRaw(this->irPower, sizeof(this->irPower), this->kFrequency);
}

void MR401::displaySettings(lv_obj_t *parent)
{
    return;
    // Serial.println("MR401 displaySettings");
    this->irCodeTable = preferences.getInt("MR401CodeTable", 0);
    // Serial.printf("irCodeTable: %d\n", this->irCodeTable);
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

    lv_obj_t *menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, "MR401");

    lv_obj_t *menuBox = lv_obj_create(parent);
    lv_obj_set_size(menuBox, lv_pct(100), 109);
    lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "IR Code");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *drop = lv_dropdown_create(menuBox);
    lv_dropdown_set_options(drop, "Option1\n"
                                  "Option2\n"
                                  "Option3\n"
                                  "Option4");
    lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, -4);
    lv_obj_set_size(drop, 100, 22);
    // lv_obj_set_style_text_font(drop, &lv_font_montserrat_12, LV_PART_MAIN);
    // lv_obj_set_style_text_font(lv_dropdown_get_list(drop), &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
    lv_obj_set_style_bg_color(drop, primary_color, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);
    uint16_t selected = 0;
    switch (this->irCodeTable)
    {
    case 1:
        selected = 1;
        break;
    case 2:
        selected = 2;
        break;
    case 3:
        selected = 3;
        break;
    default:
        break;
    }
    lv_dropdown_set_selected(drop, selected);
    lv_obj_add_event_cb(drop, mr401_dropdown_event_cb, LV_EVENT_ALL, &this->irCodeTable);
}