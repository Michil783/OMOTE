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

#pragma once
#include "HardwareAbstract.hpp"
#include <DeviceInterface.hpp>
#include <string>

class SamsungTV : public DeviceInterface {
    public:
    SamsungTV(std::shared_ptr<DisplayAbstract> display);
    //static std::shared_ptr<SamsungTV> getInstance() {return std::shared_ptr<SamsungTV>(SamsungTV::mInstance);}
    ~SamsungTV() {};
    std::string getName(){return "Samsung TV";};
    void displaySettings(lv_obj_t *menu, lv_obj_t *parent);
    void saveSettings() {};
    void handleCustomKeypad(int keyCode, char keyChar);
    bool isPowerOn();
    bool powerOn();
    bool powerOff();
private:
    std::shared_ptr<DisplayAbstract> mDisplay;
    static SamsungTV *mInstance;
    uint64_t getValue(char keyChar);

    static void virtualKeypad_event_cb(lv_event_t *e);

    static lv_obj_t *mControlPage;
    lv_obj_t *createControlPage(lv_obj_t *menu);
    lv_color_t mPrimaryColor;

#define SAMSUNGKEYS 5
    struct keys
    {
        const std::string keySymbol;
        char key;
        int col;
        int colSpan;
        int row;
        int rowSpan;
    } keyInfo[SAMSUNGKEYS] = {
        {"Power", 'p', 2, 1, 0, 1},
        {"HDMI 1", '1', 0, 1, 1, 1},
        {"HDMI 2", '2', 0, 1, 2, 1},
        {"HDMI 3", '3', 0, 1, 3, 1},
        {"HDMI 4", '4', 0, 1, 4, 1}
    };

    void MessageSend(std::string message);
    void createMagicPacket(std::string macaddress);
    void sendMagicPacket(std::string mac);

    unsigned char magicPacket[6 + (6*16)];

    struct SamsungKeys
    {
        char key;
        uint32_t code;
    } samsungKeys[5] =
        {
            {'+', 0xE0E0E01F},
            {'-', 0xE0E0D02F},
            {'o', 0xE0E040BF},
            {'m', 0xE0E0F00F},
            {'p', 0x00000000}
        };
    std::string ip = "192.168.178.43";
    std::string mac = "68:72:c3:62:53:ab";

    static lv_obj_t *mSubPage;
    static lv_obj_t *mContent;
};
