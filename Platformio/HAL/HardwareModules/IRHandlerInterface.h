#pragma once
#include "Notification.hpp"

class IRHandlerInterface {
public:
    IRHandlerInterface() = default;
    virtual void IRSender(int currentDevice, uint16_t data) = 0;
    virtual bool IRReceiverEnable(bool onoff) = 0;
    virtual bool IRReceiver() = 0;

};