/**
 * @file SamsungTV.hpp
 * @author Michael Leopoldseder
 * @brief Header file describing the device for MagentaTV Media Receiver MR401
 * @version 0.1
 * @date 2024-01-16
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include <DeviceInterface.hpp>

#ifndef _SAMSUNGTV_HPP_
#define _SAMSUNGTV_HPP_

class SamsungTV : public DeviceInterface {
    public:
    SamsungTV(Display *display);
    String getName(){return "SamsungTV";};
    void displaySettings(lv_obj_t *parent);
    void saveSettings() {};
    void handleCustomKeypad(int keyCode, char keyChar);
private:
    Display *display;
    uint64_t getValue(char keyChar);
    struct SamsungKeys
    {
        char key;
        uint64_t code;
    } samsungKeys[4] =
        {
            {'+', 0xE0E0E01F},
            {'-', 0xE0E0D02F},
            {'o', 0xE0E040BF},
            {'m', 0xE0E0F00F}
        };
    String ip = "192.168.178.43";
};

#endif
