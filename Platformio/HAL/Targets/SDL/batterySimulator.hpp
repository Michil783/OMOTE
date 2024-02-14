#include "BatteryInterface.h"
#include <chrono>
#include <cmath>
class BatterySimulator: public BatteryInterface{
    public:
        BatterySimulator() : 
            mCreationTime(std::chrono::system_clock::now())
        {};
        ~BatterySimulator(){}

        virtual int getPercentage() override { 
            auto now = std::chrono::system_clock::now();
            auto batteryRunTime = std::chrono::duration_cast<std::chrono::seconds>(now - mCreationTime);
            constexpr auto minToBatteryZero = 3;
            auto fakeBattPercentage = 100 - ((batteryRunTime / std::chrono::duration<float,std::ratio<60LL>>(minToBatteryZero)) * 100);
            if( fakeBattPercentage < 0 ) { fakeBattPercentage = 0; mCreationTime = (std::chrono::system_clock::now());}
            return std::floor(fakeBattPercentage < 100 ? fakeBattPercentage : 0);
        }

        virtual bool isCharging() override { return true; }

        virtual bool isConnected() override { return false; }

    private:
        std::chrono::system_clock::time_point mCreationTime;
};