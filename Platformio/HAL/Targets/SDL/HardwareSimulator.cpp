#include "HardwareSimulator.hpp"

#include "SDLDisplay.hpp"
#include <sstream>

HardwareSimulator::HardwareSimulator()
    : HardwareAbstract(), mTickThread([]() {
        while (true) {
          std::this_thread::sleep_for(std::chrono::milliseconds(2));
          lv_tick_inc(2); /*Tell lvgl that 2 milliseconds were elapsed*/
        }
      }),
      mBattery(std::make_shared<BatterySimulator>()),
      mDisplay(SDLDisplay::getInstance()),
      mWifiHandler(std::make_shared<wifiHandlerSim>()),
      mIRHandler(std::make_shared<IRHandlerSimulator>()),
      mKeys(std::make_shared<KeyPressSim>()) {
        mHardwareStatusTitleUpdate = std::thread([this] {
          int dataToShow = 0;
          while (true) {
            std::stringstream title;
            switch (dataToShow) {
            case 0:
              title << "Batt:" << mBattery->getPercentage() << "%" << std::endl;
              break;
            case 1:
              title << "BKLght: " << static_cast<int>(mDisplay->getBrightness())
                    << std::endl;
              dataToShow = -1;
              break;
            default:
              dataToShow = -1;
            }
            dataToShow++;

            mDisplay->setTitle(title.str());
            std::this_thread::sleep_for(std::chrono::seconds(2));
          }
        });
        mSleepTimeout = 10000;
}

std::shared_ptr<BatteryInterface> HardwareSimulator::battery() {
  return mBattery;
}
std::shared_ptr<DisplayAbstract> HardwareSimulator::display() {
  return mDisplay;
}
std::shared_ptr<wifiHandlerInterface> HardwareSimulator::wifi() {
  return mWifiHandler;
}
std::shared_ptr<KeyPressAbstract> HardwareSimulator::keys() { return mKeys; }

std::shared_ptr<IRHandlerInterface> HardwareSimulator::irhandler() { return mIRHandler; }

char HardwareSimulator::getCurrentDevice() { return 0; }

void HardwareSimulator::setCurrentDevice(char currentDevice) {}

bool HardwareSimulator::getWakeupByIMUEnabled() { return true; }

void HardwareSimulator::setWakeupByIMUEnabled(bool wakeupByIMUEnabled) {}

uint32_t HardwareSimulator::getSleepTimeout() { return mSleepTimeout; }

void HardwareSimulator::setSleepTimeout(uint32_t sleepTimeout) { mSleepTimeout = sleepTimeout; }
