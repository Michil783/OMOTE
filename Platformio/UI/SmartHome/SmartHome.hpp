#include <Display.hpp>

#ifndef _SMARTHOME_HPP_
#define _SMARTHOME_HPP_

class SmartHome : public AppInterface
{
public:
    SmartHome(Display *display);
    void setup();
    String getName();
    void handleCustomKeypad(int keyCode, char keyChar){};

private:
    /**
     * @brief pointer to Display object
     *
     */
    Display *display;

    /**
     * @brief tab object
     *
     */
    lv_obj_t *tab;

    /**
     * @brief setup SmartHome main page
     *
     * @param lv_obj_t* parent
     */
    void setup_smarthome(lv_obj_t *parent);
};

#endif