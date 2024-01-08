#include <Settings.hpp>
#include <Display.hpp>
#include <Arduino.h>

LV_IMG_DECLARE(high_brightness);
LV_IMG_DECLARE(low_brightness);

extern bool wakeupByIMUEnabled;
extern Display display;

void WakeEnableSetting_event_cb(lv_event_t * e);
void bl_slider_event_cb(lv_event_t * e);

Settings::Settings(Display* display)
{
    this->display = display;
}

void Settings::setup()
{
    Serial.println("Settings::setup()");
    this->tab = this->display->addTab("mySettings");

    /* Create main page for settings this->settingsMenu*/
    this->setup_settings(this->tab);
}

void Settings::setup_settings(lv_obj_t* parent){
  // Add content to the settings tab
  // With a flex layout, setting groups/boxes will position themselves automatically
  lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);
  // Add a label, then a box for the display settings
  this->settingsMenu = lv_menu_create(parent);
  lv_obj_set_width(this->settingsMenu, 210);

  /* Create main page for settings this->settingsMenu*/
  this->settingsMainPage = lv_menu_page_create(this->settingsMenu, NULL);
  lv_obj_t* cont = lv_menu_cont_create(this->settingsMainPage);
  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_ACTIVE);
  //lv_obj_set_width(cont, lv_obj_get_width(parent));
  this->display_settings(cont);

  //this->create_wifi_settings(this->settingsMenu, cont);

  lv_menu_set_page(this->settingsMenu, this->settingsMainPage);
}

void Settings::display_settings(lv_obj_t* parent)
{
  lv_color_t primary_color = this->display->getPrimaryColor();
  unsigned int backlight_brightness = this->display->getBacklightBrightness();

  lv_obj_t* menuLabel = lv_label_create(parent);
  lv_label_set_text(menuLabel, "Display");
  
  lv_obj_t* menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 109);
  lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  lv_obj_t* brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &low_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_t* slider = lv_slider_create(menuBox);
  lv_slider_set_range(slider, 30, 255);
  lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
  lv_slider_set_value(slider, backlight_brightness, LV_ANIM_OFF);
  lv_obj_set_size(slider, lv_pct(66), 10);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 3);
  brightnessIcon = lv_img_create(menuBox);
  lv_img_set_src(brightnessIcon, &high_brightness);
  lv_obj_set_style_img_recolor(brightnessIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(brightnessIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(brightnessIcon, LV_ALIGN_TOP_RIGHT, 0, -1);
  lv_obj_add_event_cb(slider, bl_slider_event_cb, LV_EVENT_VALUE_CHANGED, &backlight_brightness);
  
  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Lift to Wake");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 32);
  lv_obj_t* wakeToggle = lv_switch_create(menuBox);
  lv_obj_set_size(wakeToggle, 40, 22);
  lv_obj_align(wakeToggle, LV_ALIGN_TOP_RIGHT, 0, 29);
  lv_obj_set_style_bg_color(wakeToggle, lv_color_hex(0x505050), LV_PART_MAIN);
  lv_obj_add_event_cb(wakeToggle, WakeEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  if(wakeupByIMUEnabled) lv_obj_add_state(wakeToggle, LV_STATE_CHECKED); // set default state

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Timeout");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 64);
  lv_obj_t* drop = lv_dropdown_create(menuBox);
  lv_dropdown_set_options(drop, "10s\n"
                                "30s\n"
                                "1m\n"
                                "3m");
  lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, 61);
  lv_obj_set_size(drop, 70, 22);
  //lv_obj_set_style_text_font(drop, &lv_font_montserrat_12, LV_PART_MAIN);
  //lv_obj_set_style_text_font(lv_dropdown_get_list(drop), &lv_font_montserrat_12, LV_PART_MAIN);
  lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
  lv_obj_set_style_bg_color(drop, primary_color, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);

}

void Settings::update()
{
}