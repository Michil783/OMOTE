//#include <Arduino.h>
#include <MR401.hpp>
#include "DisplayAbstract.h"

//#include <omote.hpp>
#include "HardwareAbstract.hpp"
#include "IRHandlerInterface.h"

#ifdef OMOTE_ESP32
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>
extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;
#endif

MR401::MR401(std::shared_ptr<DisplayAbstract> display)
{
    mDisplay = display;
}

int MR401::getValues(char keyChar){
  for (size_t i = 0; i < LIRCENTRIES; i++)
  {
    if ( this->lircKeys[i].key == keyChar )
    {
      LV_LOG_USER("key %c found: size of data is %d", this->lircKeys[i].key, this->lircKeys[i].size);
      LV_LOG_USER("buffer: %p", this->lircKeys[i].buf);
      return i;
    }
  }
  return -1;
}

void MR401::dumpBuffer(uint16_t* buf, size_t size){
    std::string buffer;
    char b[5];
    for(size_t i = 0; i < size; i++){
        buffer = buffer + std::to_string(buf[i]);
    }
    LV_LOG_TRACE("buffer={%s}", buf);
}

void MR401::handleCustomKeypad(int keyCode, char keyChar){
    LV_LOG_USER("MR401");
    LV_LOG_USER("handleCustomKeypad(%d, %c)", keyCode, keyChar);
    
    int result = getValues(keyChar);
    if( result >= 0  ){
        LV_LOG_USER("sending IR command %c size: %d buf: %p", keyChar, this->lircKeys[result].size, this->lircKeys[result].buf);
        //this->dumpBuffer(this->lircKeys[result].buf, this->lircKeys[result].size);
        #ifdef OMOTE_ESP32
        IrSender.sendRaw(this->lircKeys[result].buf, this->lircKeys[result].size, this->kFrequency);
        #endif
    }
}
