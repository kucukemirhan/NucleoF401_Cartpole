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

void DCmotor::setSpeed(float rpm) {
    // rpm değerini -200 ile 200 arasında sınırlıyoruz.
    if (rpm > 100.0f) {
        rpm = 100.0f;
    } else if (rpm < -100.0f) {
        rpm = -100.0f;
    }
    
    // Duty cycle hesaplaması: |rpm| değerini 0-100 aralığına eşliyoruz.
    float dutyCycle = (std::abs(rpm) * 100) / 100.0f;
    
    // Eğer rpm pozitifse, ileri (forward) PWM'yi aktif ediyoruz,
    // eğer negatifse, geri (reverse) PWM'yi aktif ediyoruz.
    if (rpm >= 0) {
        forwardPWM.setDutyCycle(dutyCycle);
        reversePWM.setDutyCycle(0); // Reverse PWM'yi kapatıyoruz.
        last_dir = 1;
    } else {
        reversePWM.setDutyCycle(dutyCycle);
        forwardPWM.setDutyCycle(0); // Forward PWM'yi kapatıyoruz.
        last_dir = 0;
    }
}

void DCmotor::updateSpeed() {
    dir = (targetSpeed >= 0) ? 1 : 0;

    if (_is_motor_running && (dir != last_dir)) {
        stop();
    }
    
    setSpeed(targetSpeed);
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

    if (_is_motor_running && (dir != last_dir)) {
        stop();
    }

    float effectiveSpeed = (dir == 1) ? targetSpeed : -targetSpeed;
    setSpeed(effectiveSpeed);
    start(dir);
}

void DCmotor::updateControl()
{
    int64_t currentPosition = getCurrentPosition();
    int64_t posError = targetPosition - currentPosition;
    
    const int32_t posTolerance = 10;
    if (std::abs(posError) <= posTolerance) {
        stop();
        return;
    }
    
    const float Kp_pos = 0.025f;  // her adım hata için 0.025 rpm

    float targetSpeed = Kp_pos * posError;
    
    if (targetSpeed > 200.0f) {
        targetSpeed = 100.0f;
    } else if (targetSpeed < -200.0f) {
        targetSpeed = -100.0f;
    }

    uint8_t dir = (posError >= 0) ? 1 : 0;
    
    if (_is_motor_running && (dir != last_dir)) {
        stop();
    }

    setSpeed(targetSpeed);
    start(dir);
}


void DCmotor::start(bool dir)
{
    if (_is_motor_running) return; // Prevent redundant starts

    if (dir) {
        reversePWM.stop();
        forwardPWM.start();
    } 
    else if (!dir) {
        forwardPWM.stop();
        reversePWM.start();
    }

    _is_motor_running = true;
}

void DCmotor::stop() 
{
    if (!_is_motor_running) return; // Prevent redundant stops

    forwardPWM.stop();
    reversePWM.stop();
    _is_motor_running = false;
}