#pragma once
#include "Notification.hpp"
#include "memory.h"
#include "wifiHandlerInterface.h"
#include "WiFi.h"
//#include <Arduino.h>
#include <string>

#ifndef _WIFIHANDLER_HPP_
#define _WIFIHANDLER_HPP_

class WifiHandler : public wifiHandlerInterface {
    public:
        static std::shared_ptr<WifiHandler> getInstance();

        // wifiHandlerInterface Implementation
        void begin() override;
        void scan() override;
        void connect(std::string ssid, std::string password) override;
        wifiStatus GetStatus() override { return mCurrentStatus; };

        // /// @brief WifiHandler class
        // WifiHandler();
        // /**
        //  * @brief Function to initialize the wifi handler 
        //  * 
        //  */
        // void begin();

        // /**
        //  * @brief function to trigger asynchronous scan for wifi networks
        //  * 
        //  */
        // void scan();

        // /**
        //  * @brief Connect to the wifi using the provided credetials 
        //  * 
        //  * @param SSID 
        //  * @param password 
        //  */
        // void connect(const char* SSID, const char* password);

        /**
         * @brief Function to disconnect from the network 
         * 
         */
        void disconnect();

        /**
         * @brief Get the SSID of the found wifi
         * 
         * @param index     index of the found wifi 
         * @return std::string   SSID of the wifi
         */
        virtual std::string getFoundSSID(unsigned int index) override;

        /**
         * @brief Get the RSSI of the found wifi
         * 
         * @param index     index of the found wifi
         * @return int      RSSI value of the found wifi 
         */
        virtual int getFoundRSSI(unsigned int index) override;

        /**
         * @brief Function to determine wether or not we are connected to a network 
         * 
         * @return true  Device is connected to wifi network
         * @return false Device is not connected to wifi network
         */
        bool isConnected();

        /**
         * @brief Function to turn off wifi
         * 
         */
        void turnOff() override;

        /**
         * @brief Function to get SSID of the currently connected wifi network
         * 
         * @return char* SSID of the currently connected network
         */
        std::string getSSID();

        // /**
        //  * @brief Function to update the wifi credentials. This function is called in the wifi event callback function
        //  * after a connection is established. Only then is the new credentials stored and the old stored credentials 
        //  * overwritten.
        //  * 
        //  * @param temporary_ssid 
        //  * @param temporary_password 
        //  */
        // void update_credetials(const char* temporary_ssid, const char* temporary_password);

        /**
         * @brief Function to get the IP address of this device 
         * 
         * @return std::string IP Address of the device
         */
        std::string getIP();
    protected:
        WifiHandler() = default;
        static std::shared_ptr<WifiHandler> mInstance;

        /**
         * @brief Function to store the credentials when we have had a
         *        successful connection
         */
        void StoreCredentials();

    private:

        /**
         * @brief Variables used to track wifi connection attempts
         */
        bool mIsConnectionAttempt = false;
        std::string mConnectionAttemptPassword;
        std::string mConnectionAttemptSSID;

        /**
         * @brief Verified Working User and Pass to Wifi network
         */
        std::string mPassword;
        std::string mSSID;

        // /**
        //  * @brief Internal variable to store the wifi password 
        //  * 
        //  */
        // char password[50];

        // /**
        //  * @brief Internal variable to store the wifi SSID 
        //  * 
        //  */
        // char SSID[50];

        /**
         * @brief Handler for incoming arduino wifi events
         * @param event - a Wifi event
         */
        void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t aEventInfo);

        /**
         * @brief Update Internal status and send out a notification
         */
        void UpdateStatus();
        wifiStatus mCurrentStatus;

};

#endif