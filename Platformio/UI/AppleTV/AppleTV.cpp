#include <AppleTV.hpp>
#include <Display.hpp>
#include <Arduino.h>

#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern IRsend IrSender;

// LVGL declarations
LV_IMG_DECLARE(appleTvIcon);
LV_IMG_DECLARE(appleDisplayIcon);
LV_IMG_DECLARE(appleBackIcon);

// Apple Key Event handler
void appleKey_event_cb(lv_event_t* e) {
  // Send IR command based on the event user data  
  IrSender.sendSony(50 + (int)e->user_data, 15);
  Serial.println(50 + (int)e->user_data);
}

AppleTV::AppleTV(Display *display)
{
    this->display = display;
}

void AppleTV::setup()
{
    Serial.println("AppleTV::setup()");
    this->tab = this->display->addTab("Apple TV");

    /* Create main page for settings this->settingsMenu*/
    this->setup_appletv(this->tab);
}

void AppleTV::setup_appletv(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

    // Add a nice apple tv logo
    lv_obj_t *appleImg = lv_img_create(parent);
    lv_img_set_src(appleImg, &appleTvIcon);
    lv_obj_align(appleImg, LV_ALIGN_CENTER, 0, -60);
    // create two buttons and add their icons accordingly
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 10, 0);
    lv_obj_set_size(button, 60, 60);
    lv_obj_set_style_radius(button, 30, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, primary_color, LV_PART_MAIN);
    lv_obj_add_event_cb(button, appleKey_event_cb, LV_EVENT_CLICKED, (void *)1);

    appleImg = lv_img_create(button);
    lv_img_set_src(appleImg, &appleBackIcon);
    lv_obj_set_style_img_recolor(appleImg, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(appleImg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(appleImg, LV_ALIGN_CENTER, -3, 0);

    button = lv_btn_create(parent);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    lv_obj_set_size(button, 60, 60);
    lv_obj_set_style_radius(button, 30, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, primary_color, LV_PART_MAIN);
    lv_obj_add_event_cb(button, appleKey_event_cb, LV_EVENT_CLICKED, (void *)2);

    appleImg = lv_img_create(button);
    lv_img_set_src(appleImg, &appleDisplayIcon);
    lv_obj_set_style_img_recolor(appleImg, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(appleImg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(appleImg, LV_ALIGN_CENTER, 0, 0);
}