#include "IRHandlerInterface.h"

#include <chrono>
#include <cmath>

class IRHandlerSimulator: public IRHandlerInterface {
    public:
        IRHandlerSimulator() { mOnOff = false; };
        ~IRHandlerSimulator(){}

        virtual void IRSender(int currentDevice, uint16_t data) override {};
        virtual bool IRReceiverEnable(bool onoff) override { mOnOff = onoff; LV_LOG_USER("IRReceiverEnable: mOnOff=%d", mOnOff); return mOnOff;};
        virtual bool IRReceiver() override { LV_LOG_USER("IRReceiver: mOnOff=%d", mOnOff); return mOnOff; };

    private:
        bool mOnOff;
};