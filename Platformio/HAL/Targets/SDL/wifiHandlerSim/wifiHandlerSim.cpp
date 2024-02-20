#include "wifiHandlerSim.hpp"
#include "lvgl.h"

using WifiInfo = wifiHandlerInterface::WifiInfo;

wifiHandlerSim::wifiHandlerSim() { LV_LOG_USER("wifiHandlerSim::wifiHandlerSim()"); }

void wifiHandlerSim::begin() { LV_LOG_USER("wifiHandlerSim::begin()"); }

static const WifiInfo wifis[] = {
    WifiInfo("High Signal Wifi", -49), WifiInfo("Mid Signal Wifi", -55),
    WifiInfo("Low Signal Wifi", -65), WifiInfo("No Signal Wifi", -90)};

void wifiHandlerSim::connect(std::string ssid, std::string password) {
  LV_LOG_USER(">>> wifiHandlerSim::connect(%s, %s)", ssid.c_str(), password.c_str());
  if (mFakeStatusThread.joinable()) {
    LV_LOG_USER("sending signal for status");
    mFakeStatusThread.join();
    //mFakeStatusThread.detach();
    mCurrentStatus.ssid = ssid;
    mCurrentStatus.isConnected = true;
    LV_LOG_USER("create thread");
    mFakeStatusThread = std::thread([this] {
      std::this_thread::sleep_for(std::chrono::seconds(2));
      mStatusUpdate->notify(mCurrentStatus);
      printf("status signal sent\n");
    });
  }
  LV_LOG_USER("<<< wifiHandlerSim::connect(%s, %s)", ssid.c_str(), password.c_str());
}

void wifiHandlerSim::scan() {
  LV_LOG_USER(">>> wifiHandlerSim::scan()");
  if (mFakeScanThread.joinable()) {
    mFakeScanThread.join();
    //mFakeScanThread.detach();
    LV_LOG_USER("create thread");
    mFakeScanThread = std::thread([this] {
      std::vector<WifiInfo> info;
      if( rand() % 10 >= 5 )
        info = std::vector(std::begin(wifis), std::end(wifis));
      std::this_thread::sleep_for(std::chrono::seconds(2));
      mScanNotification->notify(info);
      printf("scan signal sent\n");
    });
  }
  LV_LOG_USER("<<< wifiHandlerSim::scan()");
}
