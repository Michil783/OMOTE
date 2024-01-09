#include <SmartHome.hpp>
#include <Arduino.h>
#include <PubSubClient.h>

extern PubSubClient client;

// LVGL declarations
LV_IMG_DECLARE(gradientLeft);
LV_IMG_DECLARE(gradientRight);
LV_IMG_DECLARE(lightbulb);

// Smart Home Toggle Event handler
void smartHomeToggle_event_cb(lv_event_t *e)
{
  char payload[8];
  if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED))
    strcpy(payload, "true");
  else
    strcpy(payload, "false");
  // Publish an MQTT message based on the event user data
  if ((int)e->user_data == 1)
    client.publish("bulb1_set", payload);
  if ((int)e->user_data == 2)
    client.publish("bulb2_set", payload);
}

// Smart Home Toggle Event handler
void smartHomeSlider_event_cb(lv_event_t *e)
{
  lv_obj_t *slider = lv_event_get_target(e);
  char payload[8];
  dtostrf(lv_slider_get_value(slider), 1, 2, payload);
  // Publish an MQTT message based on the event user data
  if ((int)e->user_data == 1)
    client.publish("bulb1_setbrightness", payload);
  if ((int)e->user_data == 2)
    client.publish("bulb2_setbrightness", payload);
}

SmartHome::SmartHome(Display *display)
{
    this->display = display;
}

void SmartHome::setup()
{
    Serial.println("SmartHome::setup()");
    this->tab = this->display->addTab("Smart Home");

    /* Create main page for settings this->settingsMenu*/
    this->setup_smarthome(this->tab);
}

void SmartHome::setup_smarthome(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

  // Add content to the smart home tab (4)
  lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_ACTIVE);

  // Add a label, then a box for the light controls
  lv_obj_t *menuLabel = lv_label_create(parent);
  lv_label_set_text(menuLabel, "Living Room");

  lv_obj_t *menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  lv_obj_t *bulbIcon = lv_img_create(menuBox);
  lv_img_set_src(bulbIcon, &lightbulb);
  lv_obj_set_style_img_recolor(bulbIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(bulbIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(bulbIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Floor Lamp");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 22, 3);
  lv_obj_t *lightToggleA = lv_switch_create(menuBox);
  lv_obj_set_size(lightToggleA, 40, 22);
  lv_obj_align(lightToggleA, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(lightToggleA, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lightToggleA, primary_color, LV_PART_INDICATOR);
  lv_obj_add_event_cb(lightToggleA, smartHomeToggle_event_cb, LV_EVENT_VALUE_CHANGED, (void *)1);

  lv_obj_t *slider = lv_slider_create(menuBox);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(lv_color_black(), 30), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(slider, lv_color_lighten(lv_palette_main(LV_PALETTE_AMBER), 180), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_dir(slider, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider, 255, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
  lv_slider_set_value(slider, 255, LV_ANIM_OFF);
  lv_obj_set_size(slider, lv_pct(90), 10);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 37);
  lv_obj_add_event_cb(slider, smartHomeSlider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)1);

  // Add another this->settingsMenu box for a second appliance
  menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  bulbIcon = lv_img_create(menuBox);
  lv_img_set_src(bulbIcon, &lightbulb);
  lv_obj_set_style_img_recolor(bulbIcon, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(bulbIcon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_align(bulbIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "Ceiling Light");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 22, 3);
  lv_obj_t *lightToggleB = lv_switch_create(menuBox);
  lv_obj_set_size(lightToggleB, 40, 22);
  lv_obj_align(lightToggleB, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_color(lightToggleB, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
  lv_obj_set_style_bg_color(lightToggleB, primary_color, LV_PART_INDICATOR);
  lv_obj_add_event_cb(lightToggleB, smartHomeToggle_event_cb, LV_EVENT_VALUE_CHANGED, (void *)2);

  slider = lv_slider_create(menuBox);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(lv_color_black(), 30), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_color(slider, lv_color_lighten(lv_palette_main(LV_PALETTE_AMBER), 180), LV_PART_INDICATOR);
  lv_obj_set_style_bg_grad_dir(slider, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(slider, 255, LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, lv_color_lighten(primary_color, 50), LV_PART_MAIN);
  lv_slider_set_value(slider, 255, LV_ANIM_OFF);
  lv_obj_set_size(slider, lv_pct(90), 10);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 37);
  lv_obj_add_event_cb(slider, smartHomeSlider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)2);

  // Add another room (empty for now)
  menuLabel = lv_label_create(parent);
  lv_label_set_text(menuLabel, "Kitchen");

  menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 79);
  lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  // Make the indicator fade out at the sides using gradient bitmaps
  lv_obj_t *img1 = lv_img_create(lv_scr_act());
  lv_img_set_src(img1, &gradientLeft);
  lv_obj_align(img1, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_size(img1, 30, 30); // stretch the 1-pixel high image to 30px
  lv_obj_t *img2 = lv_img_create(lv_scr_act());
  lv_img_set_src(img2, &gradientRight);
  lv_obj_align(img2, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_size(img2, 30, 30);
}