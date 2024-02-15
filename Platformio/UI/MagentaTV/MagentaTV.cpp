#include <MagentaTV.hpp>
//#include <Display.hpp>
//#include <Arduino.h>

#define MAX_IMAGE_WIDTH 240 // Adjust for your images

#ifdef OMOTE_ESP32
#include <Preferences.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
extern IRsend IrSender;
//extern Preferences preferences;
extern void lv_port_littlefs_init();
#endif

// extern Settings settings;
// extern long standbyTimer;
// extern long standbyTimerConfigured;

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
    const std::string channelName;
    std::string fileName;
} channelInfo[SENDERICONS] = {
    {&ARD_Logo, 1, "ARD", "L:/ard.jpg"},
    {&zdf_de, 2, "ZDF", "L:/zdf.jpg"},
    {&br, 3, "BR", "L:/br.jpg"},
    {&super_rtl, 4, "S-RTL", "L:/srtl.jpg"},
    {&rtl, 5, "RTL", "L:/rtl.jpg"},
    {&sat1, 6, "SAT1", "L:/sat1.jpg"},
    {&prosieben, 7, "Pro7", "L:/pro7.jpg"},
    {&vox_de, 8, "VOX", "L:/vox.de"},
    {&rtl2, 10, "RTL2", "L:/rtl2.jpg"},
    {&syfy, 71, "SyFy", "L:/syfy.jpg"},
    {&warner, 81, "Warner\nTV", "L:/warner.jpg"},
    {&sixx, 45, "SIXX", "L:/sixx.jpg"},
    {&tele5_de, 17, "Tele5", "L:/tele5.jpg"},
    {&universal, 83, "Universal", "L:/universal.jpg"},
    {&warnerserie, 82, "Warner\nSerie", "L:/wserie.jpg"},
    {&warnertv, 81, "Warner\nComedy", "L:/wcomedy.jpg"},
    {&dmax_de, 38, "DMAX", "L:/dmax.jpg"},
    {&kabel_eins, 9, "Kabel 1", "L:/kabel1.jpg"},
    {&RTL_Crime, 70, "RTL\nCrime", "L:/crime.jpg"},
    {&rtl_nitro_de, 41, "nitro", "L:/nitro.jpg"},
    {&tlc, 46, "TLC", "L:/tlc.jpg"}};

std::shared_ptr<MR401> MagentaTV::mMR401;
std::shared_ptr<SamsungTV> MagentaTV::mSamsung;
MagentaTV* MagentaTV::mInstance;
std::shared_ptr<UI::Basic::OmoteUI> MagentaTV::mOmoteUI;

// Virtual Keypad Event handler
void MagentaTV::virtualKeypad_event_cb(lv_event_t *e)
{
    LV_LOG_USER("virtualKeypad_event_cb");
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *cont = lv_event_get_current_target(e);
    if (target == cont)
    {
        LV_LOG_USER("virtualKeypad_event_cb - container was clicked");
        return; // stop if container was clicked
    }
    LV_LOG_USER("button pressed %d", *((int*)target->user_data));
    // Send IR command based on the button user data
    if (*((int*)target->user_data) < 10)
    {
        LV_LOG_USER("Sender < 10");
        char value = (char)('0' + *((int*)target->user_data));
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
    }
    else if (*((int*)target->user_data) < 100)
    {
        LV_LOG_USER("Sender < 100");
        char value = (char)('0' + (*((int*)target->user_data) / 10));
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + (*((int*)target->user_data) % 10));
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
    }
    else
    {
        LV_LOG_USER("Sender >= 100");
        char value = (char)('0' + (*((int*)target->user_data) / 100));
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + ((*((int*)target->user_data) % 100)) / 10);
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
        value = (char)('0' + ((*((int*)target->user_data) % 100)) % 10);
        LV_LOG_USER("sending %c", value);
        mMR401->handleCustomKeypad(-1, value);
    }
    LV_LOG_USER("sending OK");
    mMR401->handleCustomKeypad(-1, 'k');
    //standbyTimer = standbyTimerConfigured;
}

void MagentaTV::fs_dropdown_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    MagentaTV *mt = (MagentaTV*)lv_event_get_user_data(e);
    //LV_LOG_USER("code: %d (%d)", code, LV_EVENT_VALUE_CHANGED);
    mt->setFontSize(lv_dropdown_get_selected(obj));
    LV_LOG_TRACE("new fontSize = %d\n", mt->getFontSize());
    mInstance->saveSettings();
    mInstance->resetMagentaTVPage();
}

/**
 * @brief Checkbox callback function for the lift to wake option
 *
 * @param e Pointer to event object for the event where this callback is called
 */
void MagentaTV::IconEnableSetting_event_cb(lv_event_t *e)
{
    // LV_LOG_USER("Settings - WakeEnableSetting_event_cb");
    *(bool*)e->user_data = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
    mInstance->saveSettings();
    mInstance->resetMagentaTVPage();
    #pragma warning("asomething is missing")
    //settings.reset_settings_menu();
}

void MagentaTV::selectColor_event_cb(lv_event_t* e){
    LV_LOG_USER("MagentaTV selectColor_ok_event");
    LV_LOG_USER("event: %d", e->code);
    lv_color_t* color = (lv_color_t*)(e->user_data);
    lv_obj_t* obj = lv_event_get_current_target(e);
    *color = lv_colorwheel_get_rgb(obj);
    mInstance->resetMagentaTVPage();
}

void MagentaTV::selectColor_ok_event_cb(lv_event_t* e){
    LV_LOG_USER("MagentaTV selectColor_ok_event");
    LV_LOG_USER("event: %d", e->code);
    lv_obj_t *obj = (lv_obj_t*)e->user_data;
    lv_obj_del(obj);
}

void MagentaTV::selectColor_close_event_cb(lv_event_t *e)
{
    LV_LOG_USER("MagentaTV selectColor_close_event");
    LV_LOG_USER("event: %d", e->code);
    lv_obj_t *obj = (lv_obj_t*)e->user_data;
    lv_obj_del(obj);
}

void MagentaTV::colorPicker_event_cb(lv_event_t *e)
{
    LV_LOG_USER("MagentaTV - colorPicker_event_cb");

    lv_obj_t * win = lv_win_create(lv_scr_act(), 20);
    lv_obj_set_size(win, 200, 240);
    lv_obj_align(win, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t * btn;
    btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 20);
    lv_obj_add_event_cb(btn, selectColor_close_event_cb, LV_EVENT_CLICKED, win);

    std::string title = "Text Color";
    if ((lv_color_t *)e->user_data == &(mInstance->bgColor))
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
    lv_colorwheel_set_rgb(cPicker, *(lv_color_t*)e->user_data);
    lv_colorwheel_set_mode(cPicker, LV_COLORWHEEL_MODE_VALUE); // LV_COLORWHEEL_MODE_HUE+LV_COLORWHEEL_MODE_SATURATION+LV_COLORWHEEL_MODE_VALUE);
    //lv_colorwheel_set_mode_fixed(cPicker, true);
    lv_obj_set_size(cPicker, 145, 145);
    lv_obj_align(cPicker, LV_ALIGN_CENTER, 0, -30);
    lv_obj_add_event_cb(cPicker, selectColor_event_cb, LV_EVENT_VALUE_CHANGED, e->user_data);
}

void MagentaTV::resetMagentaTVPage()
{
    LV_LOG_USER("MagentaTV::resetMagentaTVPage()");
    lv_obj_clean(tab);
    mInstance->setup_MagentaTV(tab);
    //lv_obj_clean(mMenuBox);
    mInstance->displaySettings(mSettingsMenu);
}

MagentaTV::MagentaTV(std::shared_ptr<DisplayAbstract> display)
{
    LV_LOG_USER(">>> MagentaTV::MagentaTV");
    mInstance = this;
    display = display;
    mMR401 = std::make_unique<MR401>(display);
    mSamsung = std::make_unique<SamsungTV>(display);
    mOmoteUI = UI::Basic::OmoteUI::getInstance();
    mMenuLabel = nullptr;
    mMenuBox = nullptr;

    setup();
    LV_LOG_USER("<<< MagentaTV::MagentaTV");
}

void MagentaTV::setup()
{
    LV_LOG_USER("");
    tab = mOmoteUI->addApp(this);
    //display->addApp(this);

    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("MagentaTV", false);
    mIconEnabled = preferences.getBool("MTVIcons", true);
    bgColor.full = preferences.getUShort("bgColor", lv_color_to16(lv_color_lighten(lv_color_hex(0x808080), 0)));
    textColor.full = preferences.getUShort("textColor", lv_color_to16(lv_color_black()));
    LV_LOG_USER("mIconEnabled: %d, bgColor: 0x%0.4X(0x%0.4X), textColor: 0x%0.4X(0x%0.4X)", 
        mIconEnabled, 
        lv_color_to16(bgColor), 
        preferences.getUShort("bgColor", lv_color_to16(lv_color_lighten(lv_color_hex(0x808080), 0))), 
        lv_color_to16(textColor), 
        preferences.getUShort("textColor", lv_color_to16(lv_color_black()))
    );
    fontSize = preferences.getInt("fontSize", 1);
    preferences.end();
    #else
    mIconEnabled = false;
    bgColor.full = lv_color_to16(lv_color_lighten(lv_color_hex(0x808080), 0));
    textColor.full = lv_color_to16(lv_color_black());
    LV_LOG_USER("mIconEnabled: %d, bgColor: 0x%0.4X, textColor: 0x%0.4X", 
        mIconEnabled, 
        lv_color_to16(bgColor), 
        lv_color_to16(textColor));
    fontSize = 1;
    #endif

    /* Create main page for the app */
    setup_MagentaTV(tab);
}

void MagentaTV::setup_MagentaTV(lv_obj_t *parent)
{
    LV_LOG_USER(">>> MagentaTV::setup_MagentaTV()");
    lv_color_t primary_color = mOmoteUI->getPrimaryColor();
    unsigned int *backlight_brightness = display->getBacklightBrightness();

    // Configure number button grid
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};                                         // equal x distribution
    static lv_coord_t row_dsc[] = {104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, LV_GRID_TEMPLATE_LAST}; // manual y distribution to compress the grid a bit

    // Create a container with grid for tab2
    lv_obj_set_style_pad_all(parent, 0, LV_PART_MAIN);
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, LV_STATE_DEFAULT);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, LV_STATE_DEFAULT);
    lv_obj_set_size(cont, 240, 270);
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t *buttonLabel;
    lv_obj_t *obj;
    static lv_style_t style;
    static lv_style_t style_pr;

        lv_style_init(&style);
        lv_style_set_bg_color(&style, bgColor);
        //lv_style_set_text_font(&style, &usedFont[fontSize]);
        //lv_style_set_text_color(&style, textColor);
        //lv_style_set_text_align(&style, LV_ALIGN_CENTER);

        lv_style_set_shadow_width(&style, 8);
        lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
        lv_style_set_shadow_ofs_y(&style, 8);

        /*Init the pressed style*/
        lv_style_init(&style_pr);

        /*Add a large outline when pressed*/
        lv_style_set_outline_width(&style_pr, 30);
        lv_style_set_outline_opa(&style_pr, LV_OPA_TRANSP);

        lv_style_set_translate_y(&style_pr, 5);
        lv_style_set_bg_color(&style_pr, bgColor);
        lv_style_set_bg_grad_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 4));

        /*Add a transition to the outline*/
        static lv_style_transition_dsc_t trans;
        static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
        lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);

        //lv_style_set_transition(&style_pr, &trans);

        static lv_style_t btn_style;
        lv_style_init(&btn_style);
        lv_style_set_text_font(&btn_style, &usedFont[fontSize]);
        lv_style_set_text_color(&btn_style, textColor);
        lv_style_set_text_align(&btn_style, LV_ALIGN_CENTER);
        lv_style_set_align(&btn_style, LV_ALIGN_CENTER);
        //lv_style_set_border_color(&btn_style, lv_color_black());
        //lv_style_set_border_width(&btn_style, 1);
        //lv_style_set_border_post(&btn_style, true);

    LV_LOG_USER("mIconEnabled %d", mIconEnabled);
    for (int i = 0; i < SENDERICONS; i++)
    {
        uint8_t col = i % 2;
        uint8_t row = i / 2;

        if (mIconEnabled) 
        {
            obj = lv_imgbtn_create(cont);
            lv_imgbtn_set_src(obj, LV_IMGBTN_STATE_RELEASED, nullptr, channelInfo[i].icon, nullptr);
        }
        else
        {
            // obj = lv_btn_create(cont);
            // // Create Labels for each button
            // //lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
            
            // buttonLabel = lv_label_create(obj);
            // lv_obj_set_width(buttonLabel, 98);
            // lv_label_set_text(buttonLabel, channelInfo[i].channelName.c_str());
            // lv_label_set_long_mode(buttonLabel, LV_LABEL_LONG_WRAP);

            // lv_obj_add_style(buttonLabel, &btn_style, LV_STATE_DEFAULT);

            obj = lv_btn_create(cont);

            // Create Labels for each button
            buttonLabel = lv_label_create(obj);
            //lv_obj_set_width(buttonLabel, 98);
            lv_obj_set_style_bg_color(obj, bgColor, LV_PART_MAIN);
            lv_label_set_text(buttonLabel, channelInfo[i].channelName.c_str());
            //lv_label_set_long_mode(buttonLabel, LV_LABEL_LONG_WRAP);
            lv_obj_add_style(buttonLabel, &btn_style, 0);
            //lv_obj_align(buttonLabel, LV_ALIGN_CENTER, 0, 0);
        }
        //lv_obj_add_style(obj, &style, LV_STATE_DEFAULT);
        //lv_obj_add_style(obj, &style_pr, LV_STATE_PRESSED);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
        lv_obj_set_user_data(obj, (void *)&(channelInfo[i].channel));
    }

    // Create a shared event for all button inside container
    lv_obj_add_event_cb(cont, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);

    LV_LOG_USER("<<< MagentaTV::setup_MagentaTV()");
}

void MagentaTV::handleCustomKeypad(int keyCode, char keyChar)
{
    /* some keys for TV before receiver */
    if (keyChar == 'o' || keyChar == '+' || keyChar == '-' || keyChar == 'm')
    {
        mSamsung->handleCustomKeypad(keyCode, keyChar);
    }

    /* all keys for receiver */
    mMR401->handleCustomKeypad(keyCode, keyChar);

    /* some keys after receiver for TV */
    if (keyChar == 'o')
        mSamsung->handleCustomKeypad(0, 'b');
}

void MagentaTV::displaySettings(lv_obj_t *parent)
{
    LV_LOG_USER(">>> MagentaTV::displaySettings()");

    mSettingsMenu = parent;
    lv_color_t primary_color = mOmoteUI->getPrimaryColor();

    lv_obj_t *menuLabel;
    if (!mMenuLabel)
    {
        LV_LOG_USER("create menuLabel");
        menuLabel = lv_label_create(parent);
        mMenuLabel = menuLabel;
        lv_label_set_text(menuLabel, getName().c_str());
    }

    if( mMenuBox )
        lv_obj_clean(mMenuBox);
    else
        mMenuBox = lv_obj_create(parent);
    lv_obj_set_size(mMenuBox, lv_pct(100), 139);
    lv_obj_set_style_bg_color(mMenuBox, primary_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(mMenuBox, 0, LV_PART_MAIN);

    menuLabel = lv_label_create(mMenuBox);
    lv_label_set_text(menuLabel, "Show Icons");
    lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);
    lv_obj_t *iconToggle = lv_switch_create(mMenuBox);
    lv_obj_set_size(iconToggle, 40, 22);
    lv_obj_align(iconToggle, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(iconToggle, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_add_event_cb(iconToggle, IconEnableSetting_event_cb, LV_EVENT_VALUE_CHANGED, &mIconEnabled);

    if (!mIconEnabled)
    {
        menuLabel = lv_label_create(mMenuBox);
        lv_label_set_text(menuLabel, "Button Color");
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 33);
        lv_obj_t *bgColorPicker = lv_btn_create(mMenuBox);
        lv_obj_set_size(bgColorPicker, 40, 26);
        lv_obj_align(bgColorPicker, LV_ALIGN_TOP_RIGHT, 0, 30);
        lv_obj_set_style_bg_color(bgColorPicker, bgColor, LV_PART_MAIN);
        lv_obj_add_event_cb(bgColorPicker, colorPicker_event_cb, LV_EVENT_CLICKED, &bgColor);
        menuLabel = lv_label_create(mMenuBox);
        lv_label_set_text(menuLabel, "Text Color");
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 60);
        lv_obj_t *textColorPicker = lv_btn_create(mMenuBox);
        lv_obj_set_size(textColorPicker, 40, 26);
        lv_obj_align(textColorPicker, LV_ALIGN_TOP_RIGHT, 0, 56);
        lv_obj_set_style_bg_color(textColorPicker, textColor, LV_PART_MAIN);
        lv_obj_add_event_cb(textColorPicker, colorPicker_event_cb, LV_EVENT_CLICKED, &textColor);
        menuLabel = lv_label_create(mMenuBox);
        lv_label_set_text(menuLabel, "Text Size");
        lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 90);
        lv_obj_t *drop = lv_dropdown_create(mMenuBox);
        lv_dropdown_set_options(drop, "small\n"
                                    "medium\n"
                                    "large");
        lv_obj_align(drop, LV_ALIGN_TOP_RIGHT, 0, 87);
        lv_obj_set_size(drop, 100, 22);
        lv_obj_set_style_pad_top(drop, 1, LV_PART_MAIN);
        lv_obj_set_style_bg_color(drop, primary_color, LV_PART_MAIN);
        lv_obj_set_style_bg_color(lv_dropdown_get_list(drop), primary_color, LV_PART_MAIN);
        lv_obj_set_style_border_width(lv_dropdown_get_list(drop), 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(lv_dropdown_get_list(drop), lv_color_hex(0x505050), LV_PART_MAIN);
        lv_dropdown_set_selected(drop, fontSize);
        lv_obj_add_event_cb(drop, fs_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, this);
    }
    else
    {
        lv_obj_add_state(iconToggle, LV_STATE_CHECKED); // set default state
    }

    LV_LOG_USER("<<< MagentaTV::displaySettings()");
}

void MagentaTV::saveSettings()
{
    LV_LOG_USER("mIconEnabled: %d, bgColor: 0x%0.4X, textColor: 0x%0.4X", mIconEnabled, lv_color_to16(bgColor), lv_color_to16(textColor));
    #ifdef OMOTE_ESP32
    Preferences preferences;
    preferences.begin("MagentaTV", false);
    preferences.putBool("MTVIcons", mIconEnabled);
    preferences.putUShort("bgColor", lv_color_to16(bgColor));
    LV_LOG_USER("saved bgColor: 0x%0.4X", preferences.getUShort("bgColor"));
    preferences.putUShort("textColor", lv_color_to16(textColor));
    LV_LOG_USER("saved textColor: 0x%0.4X", preferences.getUShort("textColor"));
    preferences.putInt("fontSize", fontSize);
    LV_LOG_USER("saved fontSize: %d", preferences.getInt("fontSize"));
    preferences.end();
    #endif
}