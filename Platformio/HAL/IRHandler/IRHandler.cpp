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

//#define _IR_ENABLE_DEFAULT_ false
//#define SEND_RAW true
//#define SEND_SAMSUNG true

#include <IRHandler.hpp>
#include <Preferences.h>
#include <omote.hpp>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

extern Preferences preferences;

const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTimeout = 15;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance; // kTolerance is normally 25%

// IR declarations
IRsend IrSender(IR_LED, true);
IRrecv IrReceiver(IR_RX, kCaptureBufferSize, kTimeout, true);
decode_results results;

IRHandler::IRHandler()
{
    this->IRReceiverEnabled = false;
}

void IRHandler::setup()
{
    Serial.println("IRHanlder::setup()");
    IrSender.begin();
    digitalWrite(IR_VCC, HIGH); // Turn on IR receiver
    IrReceiver.enableIRIn();
    IrReceiver.disableIRIn();    // Stop the receiver

    /* clear handler list */
    for (int i = 0; i < NUMBER_OF_HANDLER; i++)
    {
        this->irp[i] = nullptr;
    }
    this->IRReceiverEnabled = preferences.getBool("IRREnabled", false);
    Serial.printf("enabled: %d\n", this->IRReceiverEnabled);
    this->IRReceiverEnable(this->IRReceiverEnabled);
}

bool IRHandler::IRReceiverEnable(bool onoff){
    Serial.printf("IRHandler::IRReceiverEnable(%d)\n", onoff);
    if( onoff ){
        Serial.println("enable IR Receiver");
        IrReceiver.enableIRIn();    // Start the receiver
        IrReceiver.setUnknownThreshold(kMinUnknownSize);
        IrReceiver.setTolerance(kTolerancePercentage); // Override the default tolerance.
        this->IRReceiverEnabled = true;
    } else {
        Serial.println("disable IR Receiver");
        IrReceiver.disableIRIn();    // Stop the receiver
        this->IRReceiverEnabled = false;
    }
    Serial.printf("save state in preferences %d\n", this->IRReceiverEnabled);
    preferences.putBool("IRREnabled", this->IRReceiverEnabled);
    Serial.printf("saved state in preferences %d\n", preferences.getBool("IRREnabled"));
    return this->IRReceiverEnabled;
}

bool IRHandler::IRReceiver(){
    Serial.printf("IRHandler::IRReceiver %d\n", this->IRReceiverEnabled);
    return this->IRReceiverEnabled;
}

bool IRHandler::addHandler(void (*func)(uint16_t data), int device)
{
    /* find free slot in handler list */
    if (irp[device] != nullptr)
        return false;
    irp[device] = func;
    return true;
}

void IRHandler::IRSender(int currentDevice, uint16_t data)
{
    /* pass data to current active application */
    if (irp[currentDevice] != nullptr)
    {
        (irp[currentDevice])(data);
    }
    else
        Serial.printf("no current device found for %d\n", currentDevice);
}