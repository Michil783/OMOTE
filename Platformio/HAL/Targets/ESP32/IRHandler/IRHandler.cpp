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
#include <lvgl.h>
#include <Preferences.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

//extern Preferences preferences;

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
    setup();
}

void IRHandler::setup()
{
    LV_LOG_TRACE("IRHanlder::setup()");
    IrSender.begin();
    digitalWrite(IR_VCC, HIGH); // Turn on IR receiver
    IrReceiver.enableIRIn();
    IrReceiver.disableIRIn();    // Stop the receiver
    /* clear handler list */
    for (int i = 0; i < NUMBER_OF_HANDLER; i++)
    {
        this->irp[i] = nullptr;
    }
    Preferences preferences;
    this->IRReceiverEnabled = preferences.getBool("IRREnabled", false);
    LV_LOG_TRACE("enabled: %d", this->IRReceiverEnabled);
    this->IRReceiverEnable(this->IRReceiverEnabled);
}

bool IRHandler::IRReceiverEnable(bool onoff){
    LV_LOG_TRACE("IRHandler::IRReceiverEnable(%d)", onoff);
    if( onoff ){
        LV_LOG_TRACE("enable IR Receiver");
        IrReceiver.enableIRIn();    // Start the receiver
        IrReceiver.setUnknownThreshold(kMinUnknownSize);
        IrReceiver.setTolerance(kTolerancePercentage); // Override the default tolerance.
    } else {
        LV_LOG_TRACE("disable IR Receiver");
        IrReceiver.disableIRIn();    // Stop the receiver
    }
    this->IRReceiverEnabled = onoff;
    LV_LOG_TRACE("save state in preferences %d", this->IRReceiverEnabled);
    Preferences preferences;
    preferences.putBool("IRREnabled", this->IRReceiverEnabled);
    LV_LOG_TRACE("saved state in preferences %d", preferences.getBool("IRREnabled"));
    return this->IRReceiverEnabled;
}

bool IRHandler::IRReceiver(){
    LV_LOG_TRACE("IRHandler::IRReceiver %d\n", this->IRReceiverEnabled);
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
        LV_LOG_TRACE("no current device found for %d\n", currentDevice);
}