#include "IRHandlerInterface.h"

#include <chrono>
#include <cmath>

class IRHandlerSimulator: public IRHandlerInterface {
    public:
        IRHandlerSimulator() {};
        ~IRHandlerSimulator(){}

        virtual void IRSender(int currentDevice, uint16_t data) override {};

    private:
};