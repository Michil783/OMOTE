#include <Arduino.h>
#include <SamsungTV.hpp>
#include <Display.hpp>
#include <Settings.hpp>
#include <Preferences.h>

#include <omote.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;
extern Settings settings;

SamsungTV::SamsungTV(Display* display){
    this->display = display;
    settings.addDevice(this);    
}

u_int64_t SamsungTV::getValue(char keyChar){
  for (size_t i = 0; i < sizeof(this->samsungKeys)/sizeof(SamsungKeys); i++)
  {
    if ( this->samsungKeys[i].key == keyChar )
    {
      LV_LOG_USER("key %c found: data is %x", this->samsungKeys[i].key, this->samsungKeys[i].code);
      return this->samsungKeys[i].code;
    }
  }
  return -1;
}

void SamsungTV::handleCustomKeypad(int keyCode, char keyChar){
    LV_LOG_USER("SamsungTV");
    LV_LOG_USER("handleCustomKeypad(%d, %c)", keyCode, keyChar);
    uint64_t code = this->getValue(keyChar);
    if( code != -1 ) IrSender.sendSAMSUNG(code);
}

void SamsungTV::displaySettings(lv_obj_t *parent){
    LV_LOG_USER("");
    lv_color_t primary_color = display->getPrimaryColor();

    lv_obj_t *menuLabel;
    menuLabel = lv_label_create(parent);
    lv_label_set_text(menuLabel, this->getName().c_str());

    lv_obj_t* menuBox = lv_obj_create(parent);
    lv_obj_set_size(menuBox, lv_pct(100), 50);
    lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "IP:");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, this->ip.c_str());
    lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 3);

}