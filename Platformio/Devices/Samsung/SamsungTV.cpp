#include <Arduino.h>
#include <SamsungTV.hpp>
#include <Display.hpp>
#include <Preferences.h>

#include <omote.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;

SamsungTV::SamsungTV(Display* display){
    this->display = display;
}

u_int64_t SamsungTV::getValue(char keyChar){
  for (size_t i = 0; i < sizeof(this->samsungKeys)/sizeof(SamsungKeys); i++)
  {
    if ( this->samsungKeys[i].key == keyChar )
    {
      Serial.printf("key %c found: data is %x\n", this->samsungKeys[i].key, this->samsungKeys[i].code);
      return this->samsungKeys[i].code;
    }
  }
  return -1;
}

void SamsungTV::handleCustomKeypad(int keyCode, char keyChar){
    Serial.println("SamsungTV");
    Serial.printf("handleCustomKeypad(%d, %c)\n", keyCode, keyChar);
    uint64_t code = this->getValue(keyChar);
    if( code != -1 ) IrSender.sendSAMSUNG(code);
}
