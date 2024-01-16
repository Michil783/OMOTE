#ifndef _IRHANDLER_HPP_
#define _IRHANDLER_HPP_

#include<Arduino.h>

#define NUMBER_OF_HANDLER 10

class IRHandler{
    public:
    IRHandler();

    void setup();
    
    bool addHandler(void (*func)(uint16_t data), int device);
    
    /**
     * @brief send IR command to certain device in handler list
     * 
     * @param int currentDevice device
     * @param uint16_t data
    */
    void IRSender(int currentDevice, uint16_t data);
    bool IRReceiverEnable(bool onoff);
    bool IRReceiver();

    private:
    void (*irp[NUMBER_OF_HANDLER]) (uint16_t data);
    bool IRReceiverEnabled;
};

#endif
