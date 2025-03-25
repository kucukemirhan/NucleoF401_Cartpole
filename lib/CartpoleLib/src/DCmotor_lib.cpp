#include "DCmotor_lib.h"

DCmotor::DCmotor(EncoderIT &encoder, TimPWM &forward, TimPWM &reverse, TimIC &capture) :
encoder(encoder),
forwardPWM(forward),
reversePWM(reverse),
captureTimer(capture),

targetPosition(0),
targetSpeed(0)
{
    // Assume the injected objects are pre-configured.
}

DCmotor::~DCmotor() {
    stop();
}

void DCmotor::setTargetPosition(int32_t position) {
    targetPosition = position;
}

void DCmotor::setTargetSpeed(float revPerMin) {
    targetSpeed = revPerMin;
}

int64_t DCmotor::getCurrentPosition() {
    return encoder.read();
}

float DCmotor::getCurrentSpeed() {
    return captureTimer.getSpeed();
}

void DCmotor::setSpeed(float rpm, bool dir) { // maps rpm to 0-100 //max 200rpm girmelisin
    if (rpm < 0.0f) {
        rpm = 0.0f;
    }
    else if (rpm > 200.0f) {
        rpm = 200.0f;
    }

    rpm = rpm * 100 / 200.0f;

    if (dir) {
        forwardPWM.setDutyCycle(rpm);
    }
    else if (!dir) {
        reversePWM.setDutyCycle(rpm);
    }
}

void DCmotor::updateSpeed() {
    float currentSpeed = getCurrentSpeed();

    float error = targetSpeed - currentSpeed;
    if (std::abs(error) <= 1) {
        this->stop();
        return;
    }

    uint8_t dir = (error >= 0) ? 1 : 0;

    // Basit oransal kontrol
    const float Kp = 1.0f;
    float command = Kp * std::abs(error);
    
    // Komut pozitifse ileri, negatifse ters yönde çalıştır
    setSpeed(command, dir);
    start(dir);
}

void DCmotor::updatePosition() {
    int64_t currentPosition = getCurrentPosition();

    int64_t error = targetPosition - currentPosition;
    if (std::abs(error) <= 10) {
        this->stop();
        return;
    }

    uint8_t dir = (error >= 0) ? 1 : 0;
    setSpeed(targetSpeed, dir);
    start(dir);
}

void DCmotor::start(bool dir)
{
    if (dir) {
        reversePWM.stop();
        forwardPWM.start();
    } 
    else if (!dir) {
        forwardPWM.stop();
        reversePWM.start();
    }
}

void DCmotor::stop() 
{
    forwardPWM.stop();
    reversePWM.stop();
}