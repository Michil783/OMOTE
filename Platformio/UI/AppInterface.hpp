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

#include <Arduino.h>
#include <Display.hpp>

#ifndef _APPINTERFACE_HPP_
#define _APPINTERFACE_HPP_

class AppInterface {
    public:
    virtual void handleCustomKeypad(int keyCode) = 0;
    virtual String getName() = 0;
};

#endif