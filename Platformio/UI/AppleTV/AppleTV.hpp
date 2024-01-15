#include <Display.hpp>

#ifndef _APPLETV_HPP_
#define _APPLETV_HPP_

class AppleTV : public AppInterface
{
public:
    AppleTV(Display *display);
    void setup();
    String getName();
    void handleCustomKeypad(int keyCode){};

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
     * @brief setup AppleTV main page
     *
     * @param lv_obj_t* parent
     */
    void setup_appletv(lv_obj_t *parent);
};

#endif