/**
 * @file wifiHandler.cpp
 * @author Thomas Bittner
 * @brief Wifi handler is a wrapper for the arduino wifi function. This wrapper provides handling of wifi credetials and
 * storing it over reset.
 * @version 0.1
 * @date 2023-08-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <WifiHandler.hpp>
#include <Settings.hpp>

/*TODO: rework to not use global variables*/
extern WifiHandler wifihandler;
extern Settings settings;
const char* temporary_ssid;
const char* temporary_password;

std::string WifiHandler::getFoundSSID(unsigned int index)
{
  return "no Wifi";
}

int WifiHandler::getFoundRSSI(unsigned int index)
{
  return 0;
}

WifiHandler::WifiHandler()
{
    this->password[0] = '\0';
    this->SSID[0] = '\0';
}


void WifiHandler::update_credetials(const char* temporary_ssid, const char* temporary_password)
{
    if (strcmp(temporary_password, this->password) != 0 || strcmp(temporary_ssid, this->SSID) != 0)
    {
        strcpy(this->password, temporary_password);
        strcpy(this->SSID, temporary_ssid);
    }
}

void WifiHandler::scan()
{
}

void WifiHandler::begin()
{
    /* If the SSID is not empty, there was a value stored in the preferences and we try to use it.*/
    /*
    if (!ssid.empty())
    {
        LV_LOG_USER("Connecting to wifi: %s", ssid);
        strcpy(this->SSID,  ssid.c_str());
        strcpy(this->password, password.c_str());
        LV_LOG_USER("this->SSID: \"%s\" (%d, %d), this->password: \"%s\"", this->SSID, std::string(this->SSID).length(), !(std::string(this->SSID).empty()), this->password);
        this->connect(this->SSID, this->password);
    }
    else
    {
        LV_LOG_USER("no SSID or password stored");
        //Set first character to \0 indicates an empty string
        this->SSID[0] = '\0';
        this->password[0] = '\0';
        WiFi.disconnect();
    }
    */
}

void WifiHandler::connect(const char* SSID, const char* password)
{
    LV_LOG_USER("ssid: \"%s\", password: \"%s\"", SSID, password);
    temporary_password = password;
    temporary_ssid = SSID;
    LV_LOG_USER("finished");
}

void WifiHandler::turnOff()
{
}

bool WifiHandler::isConnected()
{
    return false;
}

std::string WifiHandler::getSSID()
{
    return this->SSID;
}

std::string WifiHandler::getIP()
{
    return "0.0.0.0";
}