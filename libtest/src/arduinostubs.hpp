
#include <stdint.h>
// Need to start thinking of including a real mocking framework

#ifndef MILLISSTUBBED
#define MILLISSTUBBED
uint32_t millisStubbed = 0;
extern "C" uint32_t millis() {
    return millisStubbed;
};

extern "C" void delay(uint16_t) {
};


int16_t analogReadStubbed = 0;
uint8_t analogReadPinStubbed = 0;
extern "C" int16_t analogRead(uint8_t pin) {
    analogReadPinStubbed = pin;
    return analogReadStubbed;
};

extern "C" void analogWriteRange(int16_t value) {

}
extern "C" void analogWriteFreq(int16_t freq) {

}

uint8_t analogWritePinStubbed = 0;
int16_t analogWriteStubbed = 0;
extern "C" void analogWrite(uint8_t pin, int16_t value) {
    analogWritePinStubbed = pin;
    analogWriteStubbed = value;
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

int digitalWriteStubbed = 0;
int digitalWritePinStubbed = 0;
extern "C" void digitalWrite(uint8_t pin, uint8_t v) {
    digitalWritePinStubbed = pin;
    digitalWriteStubbed  = v;
}

extern "C" void ledcSetup(uint8_t p_pwmChannel, uint16_t PWM_FREQUENCY, uint16_t PWM_RESOLUTION) {

}
extern "C" void ledcAttachPin(uint8_t p_pin, uint8_t p_pwmChannel) {

}
extern "C" void  ledcWrite(uint8_t m_pwmChannel, uint8_t pwmValue) {
    
}

#endif
