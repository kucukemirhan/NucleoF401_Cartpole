#include "DCmotor_lib.h"

DCmotor::DCmotor(EncoderIT &enc, TimPWM &pwmTimer, DigitalOut &dir)
    : encoder(enc),
      pwm(pwmTimer),
      direction(dir),
      targetPosition(0),
      speed(0),
      isPwmRunning(false)
{
    // Assume the injected objects are pre-configured.
}

DCmotor::~DCmotor() {
    stop();
}

void DCmotor::setTargetPosition(int32_t position) {
    targetPosition = position;
}

void DCmotor::setSpeed(uint16_t stepsPerSecond) {
    speed = stepsPerSecond;
    pwm.setFrequency(stepsPerSecond);
}

int32_t DCmotor::getCurrentPosition() {
    return encoder.read();
}

void DCmotor::update() {
    int32_t currentPos = getCurrentPosition();

    if (currentPos < targetPosition) {
        // For forward movement: set the proper direction.
        direction.write(GPIO_PIN_SET);
        pwm.setFrequency(speed);
        // Only start PWM if it isn't already running.
        if (!isPwmRunning) {
            pwm.start();
            isPwmRunning = true;
        }
    }
    else if (currentPos > targetPosition) {
        // For reverse movement: set the proper direction.
        direction.write(GPIO_PIN_RESET);
        pwm.setFrequency(speed);
        // Only start PWM if it isn't already running.
        if (!isPwmRunning) {
            pwm.start();
            isPwmRunning = true;
        }
    }
    else {
        // Target position reached: stop PWM if it is running.
        if (isPwmRunning) {
            pwm.stop();
            isPwmRunning = false;
        }
    }
}

void DCmotor::stop() {
    if (isPwmRunning) {
        pwm.stop();
        isPwmRunning = false;
    }
}
