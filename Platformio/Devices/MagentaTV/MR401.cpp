// #include <Arduino.h>
#include <MR401.hpp>
#include <DisplayAbstract.h>
#include <Settings.hpp>
#include "HardwareAbstract.hpp"
#include "IRHandlerInterface.h"

#ifdef OMOTE_ESP32
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>
extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;
#endif

void MR401::virtualKeypad_event_cb(lv_event_t *e)
{
    LV_LOG_USER("virtualKeypad_event_cb");
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *cont = lv_event_get_current_target(e);
    if (target == cont)
    {
        LV_LOG_USER("virtualKeypad_event_cb - container was clicked");
        return; // stop if container was clicked
    }
    LV_LOG_USER("button pressed %c", *((char*)target->user_data));
    // Send IR command based on the button user data
    mInstance->handleCustomKeypad(-1, *((char*)target->user_data));
}

MR401 *MR401::mInstance;

MR401::MR401(std::shared_ptr<DisplayAbstract> display)
{
  LV_LOG_USER(">>> MR401::MR401()");
  mDisplay = display;
  mInstance = this;
  UI::Basic::OmoteUI::getInstance()->addDevice(this);
  LV_LOG_USER("<<< MR401::MR401()");
}

int MR401::getValues(char keyChar)
{
  for (size_t i = 0; i < LIRCENTRIES; i++)
  {
    if (lircKeys[i].key == keyChar)
    {
      LV_LOG_TRACE("key %c found: size of data is %d", this->lircKeys[i].key, this->lircKeys[i].size);
      LV_LOG_TRACE("buffer: %p", this->lircKeys[i].buf);
      return i;
    }
  }
  return -1;
}

void MR401::dumpBuffer(uint16_t *buf, size_t size)
{
  std::string buffer;
  char b[5];
  for (size_t i = 0; i < size; i++)
  {
    buffer = buffer + std::to_string(buf[i]);
  }
  LV_LOG_TRACE("buffer={%s}", buf);
}

void MR401::handleCustomKeypad(int keyCode, char keyChar)
{
  LV_LOG_TRACE(">>> MR401::handleCustomKeypad(%d, %c)", keyCode, keyChar);
  LV_LOG_TRACE("handleCustomKeypad(%d, %c)", keyCode, keyChar);

  int result = getValues(keyChar);
  if (result >= 0)
  {
    LV_LOG_TRACE("sending IR command %c size: %d buf: %p", keyChar, mInstance->lircKeys[result].size, mInstance->lircKeys[result].buf);
// this->dumpBuffer(mInstance->lircKeys[result].buf, mInstance->lircKeys[result].size);
#ifdef OMOTE_ESP32
    IrSender.sendRaw(mInstance->lircKeys[result].buf, mInstance->lircKeys[result].size, mInstance->kFrequency);
#endif
  }
  LV_LOG_TRACE("<<< MR401::handleCustomKeypad()");
}

lv_obj_t *MR401::mControlPage;

void MR401::displaySettings(lv_obj_t *menu, lv_obj_t *parent)
{
  LV_LOG_USER(">>> MR401::displaySettings()");
  mPrimaryColor = UI::Basic::OmoteUI::getInstance()->getPrimaryColor();
  lv_obj_t *menuLabel;

  menuLabel = lv_label_create(parent);
  lv_label_set_text(menuLabel, this->getName().c_str());

  lv_obj_t *menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 65);
  lv_obj_set_style_bg_color(menuBox, mPrimaryColor, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  static lv_style_t btn_style;
  lv_style_init(&btn_style);
  lv_style_set_text_font(&btn_style, lv_font_default());
  lv_style_set_text_color(&btn_style, lv_color_black());
  lv_style_set_text_align(&btn_style, LV_ALIGN_CENTER);
  lv_style_set_align(&btn_style, LV_ALIGN_CENTER);

  lv_obj_t *button = lv_btn_create(menuBox);
  lv_obj_set_width(button, lv_pct(100));
  lv_obj_t *buttonLabel = lv_label_create(button);
  lv_obj_set_style_bg_color(button, lv_color_make(128, 128, 128), LV_PART_MAIN);
  lv_label_set_text(buttonLabel, "MR401");
  lv_obj_add_style(buttonLabel, &btn_style, 0);
  lv_obj_align(button, LV_ALIGN_TOP_LEFT, 0, 3);

  mControlPage = createControlPage(menu);
  lv_menu_set_load_page_event(menu, button, mControlPage);

  LV_LOG_USER("<<< MR401::displaySettings()");
}

lv_obj_t *MR401::createControlPage(lv_obj_t *menu)
{
  LV_LOG_USER(">>> MR401::createControlPage()");
  lv_obj_t *menuLabel;

  lv_obj_t *ret_val = lv_menu_page_create(menu, NULL);
  lv_obj_t *cont = lv_menu_cont_create(ret_val);
  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);

  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
  static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};              // manual y distribution to compress the grid a bit

  // Create a container with grid for tab2
  lv_obj_set_style_pad_all(ret_val, 0, LV_PART_MAIN);
  lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, LV_STATE_DEFAULT);
  lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, LV_STATE_DEFAULT);
  lv_obj_set_size(cont, lv_pct(98), 10 * 60);
  lv_obj_center(cont);
  lv_obj_set_layout(cont, LV_LAYOUT_GRID);

  lv_obj_t *buttonLabel;
  lv_obj_t *obj;
  static lv_style_t style;

  lv_style_init(&style);
  lv_style_set_bg_color(&style, mPrimaryColor);

  static lv_style_t btn_style;
  lv_style_init(&btn_style);
  lv_style_set_text_font(&btn_style, lv_font_default());
  lv_style_set_text_color(&btn_style, lv_color_black());
  lv_style_set_text_align(&btn_style, LV_ALIGN_CENTER);
  lv_style_set_align(&btn_style, LV_ALIGN_CENTER);
  //lv_style_set_bg_color(&btn_style, lv_color_make(120, 120, 120));

  for (int i = 0; i < MR401KEYS; i++)
  {
    LV_LOG_USER("key: %d col: %d, row: %d", i, keyInfo[i].col, keyInfo[i].row);
    obj = lv_btn_create(cont);
    lv_obj_set_style_bg_color(obj, lv_color_make(120, 120, 120), LV_PART_MAIN);
    // Create Labels for each button
    buttonLabel = lv_label_create(obj);
    lv_label_set_text(buttonLabel, keyInfo[i].keySymbol.c_str());
    lv_obj_add_style(buttonLabel, &btn_style, 0);

    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, keyInfo[i].col, keyInfo[i].colSpan, LV_GRID_ALIGN_STRETCH, keyInfo[i].row, keyInfo[i].rowSpan);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
    lv_obj_set_user_data(obj, (void *)&(keyInfo[i].key));
  }
  // Create a shared event for all button inside container
  lv_obj_add_event_cb(cont, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);

  LV_LOG_USER("<<< MR401::createControlPage()");
  return ret_val;
}