/**
 * @file IRHandler.cpp
 * @author Michael Leopoldseder
 * @brief IR wrapper
 * @version 0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <IRHandler.hpp>
#include <Preferences.h>
#include <omote.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

// IR declarations
IRsend IrSender(IR_LED, true);
IRrecv IrReceiver(IR_RX);

IRHandler::IRHandler(){

}

void IRHandler::setup(){
  IrSender.begin();
  digitalWrite(IR_VCC, HIGH); // Turn on IR receiver
  IrReceiver.enableIRIn();    // Start the receiver
  /* clear handler list */
    for(int i = 0; i < NUMBER_OF_HANDLER; i++){
        this->irp[i] = nullptr;
    }
}

bool IRHandler::addHandler(void (*func)(uint16_t data), int device){
    /* find free slot in handler list */
    if( irp[device] != nullptr ) return false;
    irp[device] = func;
    return true;
}

void IRHandler::IRSender(int currentDevice, uint16_t data){
    /* pass data to current active application */
    if( irp[currentDevice] != nullptr ){
        (irp[currentDevice])(data);
    }
    else Serial.printf("no current device found for %d\n", currentDevice);
}