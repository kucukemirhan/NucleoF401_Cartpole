#ifndef INC_ENCODER_LIB_H_
#define INC_ENCODER_LIB_H_

#include "tim.h"
#include "ISR_obj.h"

#define ENCODER_COUNT (4) // encoders are configured along timers, so we have 4 of them too

class EncoderBase {
public:
    EncoderBase(TIM_HandleTypeDef *htim, uint32_t Channel = TIM_CHANNEL_ALL);
    ~EncoderBase(void);

    virtual HAL_StatusTypeDef start(void);
    virtual HAL_StatusTypeDef stop(void);
    void reset(void);
    uint32_t read(void);

protected:
    TIM_HandleTypeDef *_htim;
    uint32_t _Channel;

};

//using interrupt to update the counter outside of its range
class EncoderIT : public EncoderBase {
public:
    EncoderIT(TIM_HandleTypeDef *htim);
    ~EncoderIT(void);

    HAL_StatusTypeDef start(void) override; 
    HAL_StatusTypeDef stop(void) override;
    int64_t read(void);

protected:
    static class ISR<EncoderIT> ISR_List;

private:
    const int32_t TIMER_MAX = 65536; // 16 bit timer, şu an işe yaramıyor belki ileride koda eklerim diye koydum
    int64_t _overflow = 0;
    uint32_t rawCount = 0;
    
    static void PeriodElapsedCallback(TIM_HandleTypeDef *htim);
    void handleOverflow(void);
};
#endif /* INC_ENCODER_LIB_H_ */