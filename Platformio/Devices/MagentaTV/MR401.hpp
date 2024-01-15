/**
 * @file MR401.hpp
 * @author Michael Leopoldseder
 * @brief Header file describing the device for MagentaTV Media Receiver MR401
 * @version 0.1
 * @date 2024-01-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
#include <DeviceInterface.hpp>

#ifndef _MR401_HPP_
#define _MR401_HPP_

class MR401 : public DeviceInterface {
    public:
    MR401(Display* display);
    String getName();
    void displaySettings(lv_obj_t *parent);
    void saveSettings();
    void handleCustomKeypad(int keyCode);

    private:
    Display* display;
    int irCodeTable;
    const uint16_t kFrequency = 38000;  // in Hz. e.g. 38kHz.
    uint16_t irPower[25] = {296, 624,  326, 288,  610, 316,  318, 578,  358, 252,  616, 618,  616, 610,  350, 252,  352, 260,  610, 618,  616, 312,  316, 584,  334};
};

#endif