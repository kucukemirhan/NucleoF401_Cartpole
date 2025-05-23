#include "TIM_lib.h"
#include "ISR_obj.h"

ISR<TimIT> TimIT::ISR_List;
ISR<TimPWM> TimPWM::ISR_List;
ISR<TimIC> TimIC::ISR_List;

void TimIT::PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    for (uint16_t i = 0; i < ISR_List.size(); i++)
    {
        if (ISR_List.get(i)->_htim == htim)
        {
            ISR_List.get(i)->tick();
        }
    }
}

void TimPWM::PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    for (uint16_t i = 0; i < ISR_List.size(); i++)
    {
        if (ISR_List.get(i)->_htim == htim)
        {
            ISR_List.get(i)->setPSC(1);
        }
    }
}

void TimIC::IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    for (uint16_t i = 0; i < ISR_List.size(); i++)
    {
        if (ISR_List.get(i)->_htim == htim)
        {
            ISR_List.get(i)->captureValue();
        }
    }
}

// Timer base implementation
TimBase::TimBase(TIM_TypeDef *Instance, TIM_HandleTypeDef *htim): _Instance(Instance), _htim(htim) 
{   
    // maybe initialize the timer here
}

TimBase::~TimBase(void)
{

}

HAL_StatusTypeDef TimBase::start(void) 
{
    return HAL_TIM_Base_Start(_htim);
}

HAL_StatusTypeDef TimBase::stop(void) 
{
    return HAL_TIM_Base_Stop(_htim);
}

void TimBase::reset(void) 
{
    _htim->Instance->CNT = 0;
}

uint32_t TimBase::read(void) 
{
    return _htim->Instance->CNT;
}

uint32_t TimBase::getTIMclock(void)
{
    if (_Instance == TIM1)
    {
        return HAL_RCC_GetPCLK2Freq();

    } else // TIM2, TIM3 or TIM4
    {
        if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        {
             return HAL_RCC_GetPCLK1Freq() * 2;
        } else
        {
            return HAL_RCC_GetPCLK1Freq(); //maybe wrong, check later
        }
    }
}

// Timer with interrupt
TimIT::TimIT(TIM_TypeDef *Instance, TIM_HandleTypeDef *htim): 
TimBase(Instance, htim), _tick(0) 
{
#if USE_HAL_TIM_REGISTER_CALLBACKS != 1
#error "USE_HAL_TIM_REGISTER_CALLBACKS must be enabled in stm32f1xx_hal_conf.h"
#endif

    htim->PeriodElapsedCallback = PeriodElapsedCallback;
    ISR_List.add(this);
}

TimIT::~TimIT(void) 
{
    ISR_List.remove(this);
}

HAL_StatusTypeDef TimIT::start(void) 
{
    return HAL_TIM_Base_Start_IT(_htim);
}

HAL_StatusTypeDef TimIT::stop(void) 
{
    return HAL_TIM_Base_Stop_IT(_htim);
}

void TimIT::reset(void) 
{
    _tick = 0;
}

uint64_t TimIT::read(void) 
{
    return _tick;
}

void TimIT::tick(void) 
{
    _tick++;
}

bool TimIT::delay_ms(uint64_t start_time, uint64_t delay_time)
{
    return read() - start_time >= delay_time;
}

// PWM timer
TimPWM::TimPWM(TIM_TypeDef *Instance, TIM_HandleTypeDef *htim, uint32_t Channel): 
TimBase(Instance, htim), _Channel(TIM_CHANNEL_1)
{
#if USE_HAL_TIM_REGISTER_CALLBACKS != 1
#error "USE_HAL_TIM_REGISTER_CALLBACKS must be enabled in stm32f1xx_hal_conf.h"
#endif

    if (!(_Instance->CR1 & TIM_CR1_ARPE)) 
    {      
        _Instance->CR1 |= TIM_CR1_ARPE;  // enable ARR preloading     
        // _Instance->EGR |= TIM_EGR_UG;    // trigger update
    }

    if (!(_Instance->CCMR1 & TIM_CCMR1_OC1PE)) // if CCR1 preload is not enabled
    {
        _Instance->CCMR1 |= TIM_CCMR1_OC1PE;  // Enable CCR1 preload
    }
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    
    htim->PeriodElapsedCallback = PeriodElapsedCallback;
    ISR_List.add(this);
}

TimPWM::~TimPWM(void) 
{
    ISR_List.remove(this);
}

HAL_StatusTypeDef TimPWM::start(void) 
{
    return HAL_TIM_PWM_Start_IT(_htim, _Channel);
}

HAL_StatusTypeDef TimPWM::stop(void) 
{
    return HAL_TIM_PWM_Stop_IT(_htim, _Channel);
}

void TimPWM::setThisARR(uint16_t arr) 
{
    _Instance->CR1 &= ~TIM_CR1_CEN; // disable tim
    _Instance->SR &= ~TIM_SR_UIF; // clear update interrupt flag
    _Instance->ARR = arr;
    _Instance->CCR1 = arr / 2;
    _Instance->DIER |= TIM_DIER_UIE; // enable update interrupt
    _Instance->CR1 |= TIM_CR1_CEN; // enable tim
}

void TimPWM::setNextARR(uint16_t arr) 
{
    _Instance->ARR = arr;
    _Instance->CCR1 = arr / 2;
}

void TimPWM::setPSC(uint16_t psc)
{
    _Instance->PSC = psc;
}

void TimPWM::setDutyCycle(float dutyCycle)
{
    if (dutyCycle < 0.0f)
    {
        dutyCycle = 0.0f;
    }
    else if (dutyCycle > 100.0f)
    {
        dutyCycle = 100.0f;
    }

    uint16_t arr = _Instance->ARR;
    uint16_t ccr_value = static_cast<uint16_t>((dutyCycle / 100.0f) * arr);
    _Instance->CCR1 = ccr_value;
}

void TimPWM::setFrequency(uint16_t frequency)
{
    if (frequency != 0)
    {
        uint16_t arr = _Instance->ARR;

        if (_Instance->CR1 & TIM_CR1_CEN) // if timer is running
        {
            if (arr <= (getTIMclock() / 250 - 1))
            {
                this->setNextARR(getTIMclock() / frequency - 1);
                return;
            }

            uint16_t new_arr = getTIMclock() / frequency - 1;
            uint16_t cnt = _Instance->CNT;
            if (cnt <= new_arr)
            {
                this->setThisARR(new_arr - cnt);
                this->setNextARR(new_arr);
            } else
            {
                this->setThisARR(new_arr);
                this->PeriodElapsedCallback(_htim); // alttaki de olabilir tam emin değilim henüz
                //_Instance->EGR |= TIM_EGR_UG; // trigger update
            }
        } else
        {
            this->setThisARR(getTIMclock() / frequency - 1);
        }
    } else
    {
        this->stop();
    }
}

void TimPWM::reset(void) 
{
    _htim->Instance->CNT = 0;
}

uint32_t TimPWM::getCNT(void) 
{
    return _htim->Instance->CNT;
}

TimIC::TimIC(TIM_TypeDef *Instance, TIM_HandleTypeDef *htim, uint32_t Channel):
TimBase(Instance, htim), _Channel(TIM_CHANNEL_1) 
{
#if USE_HAL_TIM_REGISTER_CALLBACKS != 1
#error "USE_HAL_TIM_REGISTER_CALLBACKS must be enabled in stm32f1xx_hal_conf.h"
#endif
    
    htim->IC_CaptureCallback = IC_CaptureCallback;
    ISR_List.add(this);
}

TimIC::~TimIC(void) 
{

}

HAL_StatusTypeDef TimIC::start(void) 
{
    return HAL_TIM_IC_Start_IT(_htim, _Channel);
}

HAL_StatusTypeDef TimIC::stop(void) 
{
    return HAL_TIM_IC_Stop_IT(_htim, _Channel);
}

void TimIC::captureValue(void) 
{
    capture = HAL_TIM_ReadCapturedValue(_htim, _Channel);
    pulsePeriod = capture - lastCapture;

    if (pulsePeriod < 0)
    {
        pulsePeriod = (int32_t)65535 - lastCapture + capture; // 16-bit timer overflow
    }

    lastCapture = capture;
}

float TimIC::getSpeed(void)
{
    if (pulsePeriod == 0)
    {
        return 0.0f;  // Return 0 speed if no valid period measured
    }

    // Timer count'u saniyeye çevir
    float pulsePeriodSec = (float)pulsePeriod * (timerTickTime_us / 1e6f);

    // Bir tam dönüş için geçen süreyi hesapla
    float revolutionTimeSec = pulsePeriodSec * pulsesPerRevolution;

    // Saniyedeki dönüş sayısını (RPS) hesapla
    float revPerSec = 1.0f / revolutionTimeSec;
    
    return revPerSec; 
}