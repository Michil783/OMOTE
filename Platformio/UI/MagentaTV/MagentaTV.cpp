#include <MagentaTV.hpp>
#include <Display.hpp>
#include <Arduino.h>
#include <Preferences.h>

#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

#define MAX_IMAGE_WIDTH 240 // Adjust for your images

extern IRsend IrSender;
extern Settings settings;
extern long standbyTimer;
extern long standbyTimerConfigured;
extern Preferences preferences;
extern void lv_port_littlefs_init();

#define SENDERICONS 21

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

struct channels
{
    const void *icon;
    int channel;
    const String channelName;
    String fileName;
} channelInfo[SENDERICONS] = {
    {&ARD_Logo, 1, "ARD", "L:/ard.jpg"},
    {&zdf_de, 2, "ZDF", "L:/zdf.jpg"},
    {&br, 3, "BR", "L:/br.jpg"},
    {&super_rtl, 4, "SuperRTL", "L:/srtl.jpg"},
    {&rtl, 5, "RTL", "L:/rtl.jpg"},
    {&sat1, 6, "SAT1", "L:/sat1.jpg"},
    {&prosieben, 7, "Pro7", "L:/pro7.jpg"},
    {&vox_de, 8, "VOX", "L:/vox.de"},
    {&rtl2, 10, "RTL2", "L:/rtl2.jpg"},
    {&syfy, 71, "SyFy", "L:/syfy.jpg"},
    {&warner, 81, "WarnerTV", "L:/warner.jpg"},
    {&sixx, 45, "SIXX", "L:/sixx.jpg"},
    {&tele5_de, 17, "Tele5", "L:/tele5.jpg"},
    {&universal, 83, "Universal", "L:/universal.jpg"},
    {&warnerserie, 82, "Warner\nSerie", "L:/wserie.jpg"},
    {&warnertv, 81, "Warner\nComedy", "L:/wcomedy.jpg"},
    {&dmax_de, 38, "DMAX", "L:/dmax.jpg"},
    {&kabel_eins, 9, "Kabel 1", "L:/kabel1.jpg"},
    {&RTL_Crime, 70, "RTL Crime", "L:/crime.jpg"},
    {&rtl_nitro_de, 41, "nitro", "L:/nitro.jpg"},
    {&tlc, 46, "TLC", "L:/tlc.jpg"}};

MR401 *globalMR401;
MagentaTV *globalMagentaTV;

// Virtual Keypad Event handler
void virtualKeypad_event_cb(lv_event_t *e)
{
    LV_LOG_USER("virtualKeypad_event_cb");
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *cont = lv_event_get_current_target(e);
    if (target == cont)
    {
        LV_LOG_USER("virtualKeypad_event_cb - container was clicked");
        return; // stop if container was clicked
    }
    LV_LOG_USER("button pressed %d", (int)target->user_data);
    // Send IR command based on the button user data
    // IrSender.sendRC5(IrSender.encodeRC5X(0x00, virtualKeyMapMagentaTV[(int)target->user_data]));
    // globalMR401->handleCustomKeypad(-1, virtualKeyMapMagentaTV[(int)target->user_data]);
    if ((int)target->user_data < 10)
    {
        LV_LOG_USER("Sender < 10");
        char value = (char)('0' + (int)target->user_data);
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
    }
    else if ((int)target->user_data < 100)
    {
        LV_LOG_USER("Sender < 100");
        char value = (char)('0' + ((int)target->user_data / 10));
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + ((int)target->user_data % 10));
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
    }
    else
    {
        LV_LOG_USER("Sender >= 100");
        char value = (char)('0' + ((int)target->user_data / 100));
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + (((int)target->user_data % 100)) / 10);
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + (((int)target->user_data % 100)) % 10);
        LV_LOG_USER("sending %c", value);
        globalMR401->handleCustomKeypad(-1, value);
    }
    LV_LOG_USER("sending OK");
    globalMR401->handleCustomKeypad(-1, 'k');
    standbyTimer = standbyTimerConfigured;
}

/**
 * @brief Checkbox callback function for the lift to wake option
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void IconEnableSetting_event_cb(lv_event_t *e)
{
    // LV_LOG_USER("Settings - WakeEnableSetting_event_cb");
    *(bool*)e->user_data = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    globalMagentaTV->saveSettings();
    globalMagentaTV->resetMagentaTVPage();
    settings.reset_settings_menu();
}

void selectColor_event_cb(lv_event_t* e){
    LV_LOG_USER("MagentaTV selectColor_ok_event");
    LV_LOG_USER("event: %d", e->code);
    lv_color_t* color = (lv_color_t*)(e->user_data);
    lv_obj_t* obj = lv_event_get_current_target(e);
    *color = lv_colorwheel_get_rgb(obj);
    globalMagentaTV->resetMagentaTVPage();
}

void selectColor_ok_event_cb(lv_event_t* e){
    LV_LOG_USER("MagentaTV selectColor_ok_event");
    LV_LOG_USER("event: %d", e->code);
    lv_obj_t *obj = (lv_obj_t*)e->user_data;
    lv_obj_del(obj);
}

void selectColor_close_event_cb(lv_event_t *e)
{
    LV_LOG_USER("MagentaTV selectColor_close_event");
    LV_LOG_USER("event: %d", e->code);
    lv_obj_t *obj = (lv_obj_t*)e->user_data;
    lv_obj_del(obj);
}

void colorPicker_event_cb(lv_event_t *e)
{
    LV_LOG_USER("MagentaTV - colorPicker_event_cb");

    lv_obj_t * win = lv_win_create(lv_scr_act(), 20);
    lv_obj_set_size(win, 200, 240);
    lv_obj_align(win, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t * btn;
    btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 20);
    lv_obj_add_event_cb(btn, selectColor_close_event_cb, LV_EVENT_CLICKED, win);

    String title = "Text Color";
    if ((lv_color_t *)e->user_data == &globalMagentaTV->bgColor)
        title = "Button Color";
    lv_win_add_title(win, title.c_str());

    lv_obj_t *cont = lv_win_get_content(win);  /*Content can be added here*/
    lv_obj_t *okBtn = lv_btn_create(cont);
    lv_obj_set_size(okBtn, 40, 40);
    lv_obj_t* label = lv_label_create(okBtn);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label, "Ok");
    lv_obj_align(okBtn, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(okBtn, selectColor_ok_event_cb, LV_EVENT_CLICKED, win);

    lv_obj_t* cPicker = lv_colorwheel_create(cont, true);
    //lv_colorwheel_set_rgb(cPicker, *(lv_color_t*)e->user_data);
    //lv_colorwheel_set_mode(cPicker, LV_COLORWHEEL_MODE_SATURATION);
    lv_colorwheel_set_mode_fixed(cPicker, true);
    lv_obj_set_size(cPicker, 145, 145);
    lv_obj_align(cPicker, LV_ALIGN_CENTER, 0, -30);
    lv_obj_add_event_cb(cPicker, selectColor_event_cb, LV_EVENT_VALUE_CHANGED, e->user_data);
}

void MagentaTV::resetMagentaTVPage()
{
    LV_LOG_USER("MagentaTV::resetMagentaTVPage()");
    lv_obj_clean(this->tab);
    this->setup_MagentaTV(this->tab);
    //lv_obj_clean(this->menuBox);
    this->displaySettings(this->settingsMenu);
}

MagentaTV::MagentaTV(Display *display)
{
    this->display = display;

    this->mr401 = new MR401(display);
    globalMR401 = this->mr401;
    globalMagentaTV = this;

    this->samsung = new SamsungTV(display);
}

void MagentaTV::setup()
{
    LV_LOG_USER("");
    this->tab = this->display->addTab(this);
    settings.addApp(this);

    iconEnabled = preferences.getBool("MTVIcons", true);
    this->bgColor.full = preferences.getUShort("bgColor", lv_color_to16(lv_color_lighten(lv_color_hex(0x808080), 0)));
    this->textColor.full = preferences.getUShort("textColor", lv_color_to16(lv_color_black()));
    LV_LOG_USER("iconEnabled: %d, bgColor: 0x%0.4X(0x%0.4X), textColor: 0x%0.4X(0x%0.4X)", 
        this->iconEnabled, 
        lv_color_to16(this->bgColor), 
        preferences.getUShort("bgColor", lv_color_to16(lv_color_lighten(lv_color_hex(0x808080), 0))), 
        lv_color_to16(this->textColor), 
        preferences.getUShort("textColor", lv_color_to16(lv_color_black()))
    );

    /* Create main page for the app */
    this->setup_MagentaTV(this->tab);
}

void MagentaTV::setup_MagentaTV(lv_obj_t *parent)
{
    lv_color_t primary_color = this->display->getPrimaryColor();
    unsigned int *backlight_brightness = this->display->getBacklightBrightness();

    // Configure number button grid
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};                                         // equal x distribution
    static lv_coord_t row_dsc[] = {104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, LV_GRID_TEMPLATE_LAST}; // manual y distribution to compress the grid a bit

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

    LV_LOG_USER("iconEnabled %d", iconEnabled);
    for (int i = 0; i < SENDERICONS; i++)
    {
        uint8_t col = i % 2;
        uint8_t row = i / 2;
        if (iconEnabled)
        {
            obj = lv_imgbtn_create(cont);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, nullptr, channelInfo[i].icon, nullptr);
        }
        else
        {
            obj = lv_btn_create(cont);
            // Create Labels for each button
            buttonLabel = lv_label_create(obj);
            lv_obj_set_style_bg_color(obj, this->bgColor, LV_PART_MAIN);
            lv_label_set_text_fmt(buttonLabel, channelInfo[i].channelName.c_str());
            static lv_style_t style;
            lv_style_init(&style);
            lv_style_set_text_font(&style, &lv_font_montserrat_16);
            lv_style_set_text_color(&style, this->textColor);
            lv_obj_add_style(buttonLabel, &style, 0);
            lv_obj_align(buttonLabel, LV_ALIGN_CENTER, 0, 0);
        }
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
        lv_obj_set_user_data(obj, (void *)channelInfo[i].channel);
    }

    // Create a shared event for all button inside container
    lv_obj_add_event_cb(cont, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);
}

void MagentaTV::handleCustomKeypad(int keyCode, char keyChar)
{
    /* some keys for TV before receiver */
    if (keyChar == 'o' || keyChar == '+' || keyChar == '-' || keyChar == 'm')
    {
        this->samsung->handleCustomKeypad(keyCode, keyChar);
    }

    /* all keys for receiver */
    this->mr401->handleCustomKeypad(keyCode, keyChar);

    /* some keys after receiver for TV */
    if (keyChar == 'o')
        this->samsung->handleCustomKeypad(0, 'b');
}

void MagentaTV::displaySettings(lv_obj_t *parent)
{
    this->settingsMenu = parent;
    lv_color_t primary_color = display->getPrimaryColor();

    lv_obj_t *menuLabel;
    if (!this->menuLabel)
    {
        menuLabel = lv_label_create(parent);
        this->menuLabel = menuLabel;
        lv_label_set_text(menuLabel, this->getName().c_str());
    }

    if( this->menuBox )
        lv_obj_clean(this->menuBox);
    else
        this->menuBox = lv_obj_create(parent);
    lv_obj_set_size(menuBox, lv_pct(100), 109);
    lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

    menuLabel = lv_label_create(menuBox);
    lv_label_set_text(menuLabel, "Show Icons");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);
    lv_obj_t *iconToggle = lv_switch_create(menuBox);
    lv_obj_set_size(iconToggle, 40, 22);
    lv_obj_align(iconToggle, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(iconToggle, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_add_event_cb(iconToggle, IconEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, &this->iconEnabled);

    if (!iconEnabled)
    {
        menuLabel = lv_label_create(menuBox);
        lv_label_set_text(menuLabel, "Button Color");
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 33);
        lv_obj_t *bgColorPicker = lv_btn_create(menuBox);
        lv_obj_set_size(bgColorPicker, 40, 26);
        lv_obj_align(bgColorPicker, LV_ALIGN_TOP_RIGHT, 0, 30);
        lv_obj_set_style_bg_color(bgColorPicker, this->bgColor, LV_PART_MAIN);
        lv_obj_add_event_cb(bgColorPicker, colorPicker_event_cb, LV_EVENT_CLICKED, &this->bgColor);
        menuLabel = lv_label_create(menuBox);
        lv_label_set_text(menuLabel, "Text Color");
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 60);
        lv_obj_t *textColorPicker = lv_btn_create(menuBox);
        lv_obj_set_size(textColorPicker, 40, 26);
        lv_obj_align(textColorPicker, LV_ALIGN_TOP_RIGHT, 0, 56);
        lv_obj_set_style_bg_color(textColorPicker, this->textColor, LV_PART_MAIN);
        lv_obj_add_event_cb(textColorPicker, colorPicker_event_cb, LV_EVENT_CLICKED, &this->textColor);
    }
    else
    {
        lv_obj_add_state(iconToggle, LV_STATE_CHECKED); // set default state
    }
}

void MagentaTV::saveSettings()
{
    LV_LOG_USER("iconEnabled: %d, bgColor: 0x%0.4X, textColor: 0x%0.4X", this->iconEnabled, lv_color_to16(this->bgColor), lv_color_to16(this->textColor));
    preferences.putBool("MTVIcons", this->iconEnabled);
    preferences.putUShort("bgColor", lv_color_to16(this->bgColor));
    LV_LOG_USER("saved bgColor: 0x%0.4X", preferences.getUShort("bgColor"));
    preferences.putUShort("textColor", lv_color_to16(this->textColor));
    LV_LOG_USER("saved textColor: 0x%0.4X", preferences.getUShort("textColor"));
}