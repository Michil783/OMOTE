#include <MagentaTV.hpp>
#include <Display.hpp>
#include <Arduino.h>

#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern IRsend IrSender;
extern Settings settings;
extern long standbyTimer;
extern long standbyTimerConfigured;

#define IMAGEBUTTONS
#ifdef IMAGEBUTTONS
LV_IMG_DECLARE(ARD_Logo);
LV_IMG_DECLARE(zdf_de);
LV_IMG_DECLARE(br);
LV_IMG_DECLARE(dmax_de);
LV_IMG_DECLARE(kabel_eins);
LV_IMG_DECLARE(prosieben);
LV_IMG_DECLARE(RTL_Crime);
LV_IMG_DECLARE(rtl_nitro_de);
LV_IMG_DECLARE(rtl);
LV_IMG_DECLARE(rtl2);
LV_IMG_DECLARE(sat1);
LV_IMG_DECLARE(sixx);
LV_IMG_DECLARE(super_rtl);
LV_IMG_DECLARE(syfy);
LV_IMG_DECLARE(tele5_de);
LV_IMG_DECLARE(tlc);
LV_IMG_DECLARE(universal);
LV_IMG_DECLARE(vox_de);
LV_IMG_DECLARE(warner);
LV_IMG_DECLARE(warnerserie);
LV_IMG_DECLARE(warnertv);
LV_IMG_DECLARE(zdf_de);
#endif

//char virtualKeyMapMagentaTV[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
#ifdef IMAGEBUTTONS
#define SENDERICONS 22
struct channels{
    const void* icon;
    int channel;
} channelInfo[SENDERICONS] = {
    {&ARD_Logo, 1},
    {&zdf_de, 2},
    {&br, 3},
    {&super_rtl, 4},
    {&rtl, 5},
    {&sat1, 6},
    {&prosieben, 7},
    {&vox_de, 8},
    {&rtl2, 10},
    {&syfy, 71},
    {&warner, 81},
    {&sixx, 45},
    {&tele5_de, 17},
    {&universal, 83},
    {&warnerserie, 82},
    {&warnertv, 81},
    {&dmax_de, 38},
    {&kabel_eins, 9},
    {&RTL_Crime, 70},
    {&rtl_nitro_de, 41},
    {&sixx, 45},
    {&tlc, 46}
};
/*
const void* senderIcons[SENDERICONS] = {&ard, &zdf_de, &br, &super_rtl, &rtl, &sat1, &prosieben, &vox_de, &rtl2, &syfy,
 &warner, &sixx, &tele5_de, &universal, &warnerserie, &warnertv, &dmax_de, &kabel_eins, &RTL_Crime, &rtl_nitro_de,
 &sixx, &tlc};
const uint16_t senderCode[SENDERICONS] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 71,
 81, 45, 17, 83, 82, 81, 38, 9, 70, 41,
 45, 46};
*/
#endif
MR401* globalMR401;

// Virtual Keypad Event handler
void virtualKeypad_event_cb(lv_event_t *e)
{
    Serial.printf("virtualKeypad_event_cb\n");
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *cont = lv_event_get_current_target(e);
    if (target == cont) {
        Serial.println("virtualKeypad_event_cb - container was clicked");
        return; // stop if container was clicked
    }
    Serial.printf("button pressed %d\n", (int)target->user_data);
    // Send IR command based on the button user data
    //IrSender.sendRC5(IrSender.encodeRC5X(0x00, virtualKeyMapMagentaTV[(int)target->user_data]));
    //globalMR401->handleCustomKeypad(-1, virtualKeyMapMagentaTV[(int)target->user_data]);
    if( (int)target->user_data < 10 ){
        Serial.println("Sender < 10");
        char value = (char)('0' + (int)target->user_data);
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
    } else if( (int)target->user_data < 100 ) {
        Serial.println("Sender < 100");
        char value = (char)('0' + ((int)target->user_data / 10));
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + ((int)target->user_data % 10));
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
    } else {
        Serial.println("Sender >= 100");
        char value = (char)('0' + ((int)target->user_data / 100));
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + (((int)target->user_data % 100)) / 10);
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + (((int)target->user_data % 100)) % 10);
        Serial.printf("sending %c\n", value);
        globalMR401->handleCustomKeypad(-1, value);
    }
    Serial.println("sending OK");
    globalMR401->handleCustomKeypad(-1, 'k');
    standbyTimer = standbyTimerConfigured;
}

MagentaTV::MagentaTV(Display *display)
{
    this->display = display;

    this->mr401 = new MR401(display);
    globalMR401 = this->mr401;

    this->samsung = new SamsungTV(display);
}

void MagentaTV::setup()
{
    Serial.println("MagentaTV::setup()");
    this->tab = this->display->addTab(this);

    /* Create main page for settings this->settingsMenu*/
    this->setup_MagentaTV(this->tab);
}

void MagentaTV::setup_MagentaTV(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

    // Configure number button grid
    //static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
    //static lv_coord_t row_dsc[] = {52, 52, 52, 52, LV_GRID_TEMPLATE_LAST};                              // manual y distribution to compress the grid a bit
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
    static lv_coord_t row_dsc[] = {104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, LV_GRID_TEMPLATE_LAST};                              // manual y distribution to compress the grid a bit

    // Create a container with grid for tab2
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_shadow_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, 240, 270);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN);

    lv_obj_t *buttonLabel;
    lv_obj_t *obj;

#ifdef IMAGEBUTTONS
    for (int i = 0; i < SENDERICONS; i++)
    {
        uint8_t col = i % 2;
        uint8_t row = i / 2;
        obj = lv_imgbtn_create(cont);
        lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, nullptr, channelInfo[i].icon, nullptr);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
        lv_obj_set_user_data(obj, (void *)channelInfo[i].channel);
    }
#else
    // Iterate through grid buttons configure them
    for (int i = 0; i < 12; i++)
    {
        uint8_t col = i % 3;
        uint8_t row = i / 3;
        // Create the button object
        if ((row == 3) && ((col == 0) || (col == 2)))
            continue; // Do not create a complete fourth row, only a 0 button
        obj = lv_btn_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_style_bg_color(obj, primary_color, LV_PART_MAIN);
        lv_obj_set_style_radius(obj, 14, LV_PART_MAIN);
        lv_obj_set_style_shadow_color(obj, lv_color_hex(0x404040), LV_PART_MAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
        // Create Labels for each button
        buttonLabel = lv_label_create(obj);
        if (i < 9)
        {
            lv_label_set_text_fmt(buttonLabel, std::to_string(i + 1).c_str(), col, row);
            lv_obj_set_user_data(obj, (void *)i); // Add user data so we can identify which button caused the container event
        }
        else
        {
            lv_label_set_text_fmt(buttonLabel, "0", col, row);
            lv_obj_set_user_data(obj, (void *)9);
        }
        lv_obj_set_style_text_font(buttonLabel, &lv_font_montserrat_24, LV_PART_MAIN);
        lv_obj_center(buttonLabel);
    }
#endif
    // Create a shared event for all button inside container
    lv_obj_add_event_cb(cont, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);
}

void MagentaTV::handleCustomKeypad(int keyCode, char keyChar){
    if( keyChar == 'o' || keyChar == '+' || keyChar == '-' || keyChar == 'm' ) {
        this->samsung->handleCustomKeypad(keyCode, keyChar);
        if( keyChar == 'o' )
            this->samsung->handleCustomKeypad(0, 'b');
    }
    this->mr401->handleCustomKeypad(keyCode, keyChar);
}

