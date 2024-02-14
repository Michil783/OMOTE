#include "Battery.hpp"

#include <Arduino.h>

//extern Display display;

Battery::Battery(int adc_pin, int charging_pin)
{
    this->adc_pin = adc_pin;
    this->charging_pin = charging_pin;
}

int Battery::getPercentage()
{
    return constrain(map(this->getVoltage(), 3700, 4200, 0, 100), 0, 100);
}

bool Battery::isCharging()
{
    LV_LOG_TRACE("isCharging: %d (pin: %d Voltage: %d)", 
        (!digitalRead(this->charging_pin) && this->getVoltage() > 4500), 
        !digitalRead(this->charging_pin), 
        this->getVoltage());
    return (!digitalRead(this->charging_pin) && this->getVoltage() > 4500);
}

bool Battery::isConnected()
{
    return (this->getVoltage() > 4500);
}

int Battery::getVoltage()
{
    return analogRead(this->adc_pin)*2*3300/4095 + 350;
}

