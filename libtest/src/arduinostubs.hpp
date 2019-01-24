
#include <stdint.h>
// Need to start thinking of including a real mocking framework

#ifndef MILLISSTUBBED
#define MILLISSTUBBED
uint32_t millisStubbed = 0;
extern "C" uint32_t millis() {
    return millisStubbed;
};


int16_t analogReadStubbed = 0;
uint8_t analogReadPinStubbed = 0;
extern "C" int16_t analogRead(uint8_t pin) {
    analogReadPinStubbed = pin;
    return analogReadStubbed;
};

int pinModeStubbed = 0;
uint8_t pinModePinStubbed = 0;
uint8_t pinModeModeStubbed = 0;
extern "C" int pinMode(uint8_t pin, uint8_t mode) {
    pinModePinStubbed = pin;
    pinModeModeStubbed = mode;
    return pinModeStubbed;
};

int digitalReadStubbed = 0;
int digitalReadPinStubbed = 0;
extern "C" int digitalRead(uint8_t pin) {
    digitalReadPinStubbed = pin;
    return digitalReadStubbed;
}


#endif
