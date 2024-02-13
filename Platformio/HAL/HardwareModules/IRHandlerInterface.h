#pragma once
#include "Notification.hpp"

class IRHandlerInterface {
public:
    IRHandlerInterface() = default;
    virtual void IRSender(int currentDevice, uint16_t data) = 0;

};