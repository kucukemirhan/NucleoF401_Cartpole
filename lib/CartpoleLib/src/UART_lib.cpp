#include "UART_lib.h"

ISR<UartIT> UartIT::ISR_LIST;

// Callback functions for UART with interrupt
void UartIT::TxCpltCallback(UART_HandleTypeDef *huart)
{
    for (uint16_t i = 0; i < ISR_LIST.size(); i++)
    {
        if (ISR_LIST.get(i)->_huart == huart)
        {
            ISR_LIST.get(i)->_is_tx_complete = true;
        }
    }
}

void UartIT::RxEventCallback(UART_HandleTypeDef *huart, uint16_t Pos)
{
    for (uint16_t i = 0; i < ISR_LIST.size(); i++)
    {
        if (ISR_LIST.get(i)->_huart == huart)
        {
            ISR_LIST.get(i)->put(0, Pos);
        }
    }
}

// UART Base functions
UartBase::UartBase(USART_TypeDef *Instance, UART_HandleTypeDef *huart):
_Instance(Instance), _huart(huart)
{

}

HAL_StatusTypeDef UartBase::write(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    return HAL_UART_Transmit(_huart, pData, Size, Timeout);
}

HAL_StatusTypeDef UartBase::read(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    return HAL_UART_Receive(_huart, pData, Size, Timeout);
}

// UART Interrupt
UartIT::UartIT(USART_TypeDef *Instance, UART_HandleTypeDef *huart) :
    UartBase(Instance, huart), _is_tx_complete(true)
{
    #if USE_HAL_UART_REGISTER_CALLBACKS != 1
    #error "USE_HAL_UART_REGISTER_CALLBACKS must be enabled in stm32f1xx_hal_conf.h"
    #endif
    
    _buffer = new CircularBuffer<uint8_t>(sizeof(_rx_buffer) * 2);
    huart->TxCpltCallback = UartIT::TxCpltCallback;
    huart->RxEventCallback = UartIT::RxEventCallback;
    ISR_LIST.add(this);

}

HAL_StatusTypeDef UartIT::write(uint8_t *pData, uint16_t Size)
{
    if (!_is_tx_complete)
    {
        return HAL_BUSY;
    }
    _is_tx_complete = false;
    return HAL_UART_Transmit_IT(_huart, pData, Size);
}

uint16_t UartIT::read(uint8_t *pData, uint16_t Size)
{
    return _buffer->pull(pData, Size);
}

HAL_StatusTypeDef UartIT::start_read(void)
{
    memset(_rx_buffer, 0, sizeof(_rx_buffer));
    return HAL_UARTEx_ReceiveToIdle_IT(_huart, _rx_buffer, sizeof(_rx_buffer));
}

void UartIT::put(uint16_t index, uint16_t size)
{
    _buffer->put(&_rx_buffer[index], size);
    start_read();
}

bool UartIT::is_tx_complete(void)
{
    return _is_tx_complete;
}