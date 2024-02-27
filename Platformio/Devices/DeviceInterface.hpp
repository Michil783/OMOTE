/**
 * @file DeviceInterface.hpp
 * @author Michael Leopoldseder
 * @brief Header file describing the interface other parts can use to access devices
 * @version 0.1
 * @date 2024-01-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//#include <Arduino.h>
//#include <Display.hpp>
#include "lvgl.h"
#include <string>

#ifndef _DEVICEINTERFACE_HPP_
#define _DEVICEINTERFACE_HPP_

class DeviceInterface {
    public:
    virtual std::string getName() = 0;
    virtual void displaySettings(lv_obj_t *menu, lv_obj_t *parent) = 0;
    virtual void saveSettings() = 0;
    virtual void handleCustomKeypad(int keyCode, char keyChar) = 0;
};

#endif