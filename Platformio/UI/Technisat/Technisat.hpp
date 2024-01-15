#include <Display.hpp>
#include <DeviceInterface.hpp>
#include <Settings.hpp>
#include <MR401.hpp>

#ifndef _TECHNISAT_HPP_
#define _TECHNISAT_HPP_

class Technisat : public AppInterface {
    public:
    Technisat(Display* display);
    void setup();
    void handleCustomKeypad(int kexCode);
    String getName();

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
     * @brief setup Technisat main page
     *
     * @param lv_obj_t* parent
     */
    void setup_technisat(lv_obj_t *parent);

    MR401* mr401;
};

#endif