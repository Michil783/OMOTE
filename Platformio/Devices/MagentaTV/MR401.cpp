#include <Arduino.h>
#include <MR401.hpp>
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

MR401::MR401(Display *display)
{
    this->display = display;
}

int MR401::getValues(char keyChar){
  for (size_t i = 0; i < LIRCENTRIES; i++)
  {
    if ( this->lircKeys[i].key == keyChar )
    {
      Serial.printf("key %c found: size of data is %d\n", this->lircKeys[i].key, this->lircKeys[i].size);
      Serial.printf("buffer: %p\n", this->lircKeys[i].buf);
      return i;
    }
  }
  return -1;
}

void MR401::dumpBuffer(uint16_t* buf, size_t size){
    Serial.print("buffer {");
    for(size_t i = 0; i < size; i++){
        Serial.printf("%d,", buf[i]);
    }
    Serial.println("");
}

void MR401::handleCustomKeypad(int keyCode, char keyChar){
    Serial.println("MR401");
    Serial.printf("handleCustomKeypad(%d, %c)\n", keyCode, keyChar);
    
    int result = getValues(keyChar);
    if( result >= 0  ){
        Serial.printf("sending IR command %c size: %d buf: %p\n", keyChar, this->lircKeys[result].size, this->lircKeys[result].buf);
        this->dumpBuffer(this->lircKeys[result].buf, this->lircKeys[result].size);
        IrSender.sendRaw(this->lircKeys[result].buf, this->lircKeys[result].size, this->kFrequency);
    }
    //IrSender.sendRaw(this->power[this->index], sizeof(this->power[this->index]), this->kFrequency);
    //this->index++; if( this->index > 11 ) this->index = 0;
}
