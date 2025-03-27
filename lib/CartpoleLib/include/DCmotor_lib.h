#ifndef INC_STEPPER_LIB_H_
#define INC_STEPPER_LIB_H_

#include "Encoder_lib.h"
#include "TIM_lib.h"
#include "GPIO_lib.h"
#include <cmath>

// This version of the DCmotor class accepts pre-constructed objects.
// It uses:
// - EncoderIT for position feedback,
// - 2 TimPWM for controlling the motor speed and direction,
// - 2 DigitalOut for controlling the motor direction enable pins.
class DCmotor {
public:
    // Constructor accepts references to already configured objects.
    DCmotor(EncoderIT &encoder, TimPWM &forwardPWM, TimPWM &reversePWM, TimIC &captureTimer);
    ~DCmotor();

    void setTargetPosition(int32_t position); // in steps
    void setTargetSpeed(float rpm); // in rpm

    // Get the current position.
    int64_t getCurrentPosition();
    float getCurrentSpeed();

    void setSpeed(float rpm, bool dir);

    // call periodically to update
    void updateSpeed();
    void updatePosition();
    void updateControl();

    void start(bool dir);
    void stop();

private:
    EncoderIT &encoder;
    TimPWM &forwardPWM;
    TimPWM &reversePWM;
    TimIC &captureTimer;

    int64_t targetPosition;  // Desired position in steps
    bool _is_motor_running;
    float targetSpeed;       // desired speed (rpm)
};

#endif // /* INC_STEPPER_LIB_H_ */