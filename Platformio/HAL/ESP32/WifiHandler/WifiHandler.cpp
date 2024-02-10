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
#include <Preferences.h>
#include <WiFi.h>
#include <Settings.hpp>

/*TODO: rework to not use global variables*/
extern WifiHandler wifihandler;
extern Preferences preferences;
//extern Display display;
extern Settings settings;
//extern PubSubClient client;
const char* temporary_ssid;
const char* temporary_password;

WiFiClient espClient;

// WiFi status event
void WiFiEvent(WiFiEvent_t event){
  int no_networks = 0;
  switch (event)
  {
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      LV_LOG_USER("WIFI scan done");
      no_networks = WiFi.scanComplete();
      if (no_networks < 0)
      {
        LV_LOG_ERROR("Scan failed");
      }
      else
      {
        settings.clear_wifi_networks();
        LV_LOG_USER("%d found", no_networks);
        settings.wifi_scan_complete( no_networks);
      }
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //client.setServer(MQTT_SERVER, 1883); // MQTT initialization
      //client.connect("OMOTE"); // Connect using a client id
      LV_LOG_USER("ARDUINO_EVENT_WIFI_STA_GOT_IP");
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      settings.update_wifi(true);
      wifihandler.update_credetials(temporary_ssid, temporary_password);
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      LV_LOG_USER("ARDUINO_EVENT_WIFI_STA_DISCONNECTED");
      settings.update_wifi(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      LV_LOG_USER("ARDUINO_EVENT_WIFI_STA_LOST_IP");
      settings.update_wifi(false);
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      LV_LOG_USER("ARDUINO_EVENT_WIFI_STA_STOP");
      settings.update_wifi(false);
      break;
    default:
      LV_LOG_USER("ARDUINO_EVENT_WIFI default");
      break;
  }
}

std::string WifiHandler::getFoundSSID(unsigned int index)
{
  return WiFi.SSID(index).c_str();
}

int WifiHandler::getFoundRSSI(unsigned int index)
{
  return WiFi.RSSI(index);
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

        preferences.putString("password", this->password);
        preferences.putString("SSID", this->SSID);
    }
}

void WifiHandler::scan()
{
    WiFi.scanNetworks(true);
}

void WifiHandler::begin()
{
    WiFi.setHostname("OMOTE");
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFiEvent);

    std::string ssid = preferences.getString("SSID", "").c_str();
    std::string password = preferences.getString("password", "").c_str();
    LV_LOG_USER("ssid: \"%s\" (%d,%d), password: \"%s\"", ssid, ssid.length(), ssid.empty(), password);

    /* If the SSID is not empty, there was a value stored in the preferences and we try to use it.*/
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
    WiFi.setSleep(true);
}

void WifiHandler::connect(const char* SSID, const char* password)
{
    LV_LOG_USER("ssid: \"%s\", password: \"%s\"", SSID, password);
    temporary_password = password;
    temporary_ssid = SSID;
    WiFi.begin(SSID, password);
    LV_LOG_USER("finished");
}

void WifiHandler::turnOff()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

bool WifiHandler::isConnected()
{
    return WiFi.isConnected();
}

std::string WifiHandler::getSSID()
{
    return this->SSID;
}

std::string WifiHandler::getIP()
{
    return (WiFi.localIP().toString()).c_str();
}