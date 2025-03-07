//#include <Arduino.h>
#include <string>

#ifndef _WIFIHANDLER_HPP_
#define _WIFIHANDLER_HPP_

class WifiHandler {
    public:
        /// @brief WifiHandler class
        WifiHandler();
        /**
         * @brief Function to initialize the wifi handler 
         * 
         */
        void begin();

        /**
         * @brief Connect to the wifi using the provided credetials 
         * 
         * @param SSID 
         * @param password 
         */
        void connect(const char* SSID, const char* password);

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
        std::string getFoundSSID(unsigned int index);

        /**
         * @brief Get the RSSI of the found wifi
         * 
         * @param index     index of the found wifi
         * @return int      RSSI value of the found wifi 
         */
        int getFoundRSSI(unsigned int index);

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
        void turnOff();

        /**
         * @brief function to trigger asynchronous scan for wifi networks
         * 
         */
        void scan();

        /**
         * @brief Function to get SSID of the currently connected wifi network
         * 
         * @return char* SSID of the currently connected network
         */
        std::string getSSID();

        /**
         * @brief Function to update the wifi credentials. This function is called in the wifi event callback function
         * after a connection is established. Only then is the new credentials stored and the old stored credentials 
         * overwritten.
         * 
         * @param temporary_ssid 
         * @param temporary_password 
         */
        void update_credetials(const char* temporary_ssid, const char* temporary_password);

        /**
         * @brief Function to get the IP address of this device 
         * 
         * @return std::string IP Address of the device
         */
        std::string getIP();
    private:

        /**
         * @brief Internal variable to store the wifi password 
         * 
         */
        char password[50];

        /**
         * @brief Internal variable to store the wifi SSID 
         * 
         */
        char SSID[50];

};

#endif