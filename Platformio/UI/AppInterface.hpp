/**
 * @file AppInterface.hpp
 * @author Michael Leopoldseder
 * @brief Header file describing the interface other parts can use to access apps
 * @version 0.1
 * @date 2024-01-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <string>
#include "lvgl.h"

#ifndef _APPINTERFACE_HPP_
#define _APPINTERFACE_HPP_

class AppInterface {
    public:
    //virtual void addAppSettings(std::shared_ptr<void> settings) = 0;
    virtual void handleCustomKeypad(int keyCode, char keyChar) = 0;
    virtual std::string getName() = 0;
    virtual void displaySettings(lv_obj_t *parent) = 0;
    virtual void saveSettings() = 0;
};

#endif