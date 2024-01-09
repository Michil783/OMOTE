#include <Display.hpp>

#ifndef _TECHNISAT_HPP_
#define _TECHNISAT_HPP_

class Technisat{
    public:
    Technisat(Display* display);
    void setup();

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
     * @brief setup settings main page
     *
     * @param lv_obj_t* parent
     */
    void setup_technisat(lv_obj_t *parent);
};

#endif