#ifndef CLI_TASK_HPP
#define CLI_TASK_HPP

#include "err_status.hpp"
#include "meas_task.hpp"
#include "cli_api.hpp"
#include "cli.hpp"
#include "uart.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <cstdint>

class CliTask : public CliControlApi
{
public:

    struct Config
    {
        uint32_t    stackSize;
        UBaseType_t priority;
    };

    ErrStatus init(Config cfg);
    ErrStatus start();
    ErrStatus stop();

    ErrStatus registerCommand(const Command& entry) override;

private:

    static void taskEntry_(void* pvParameters);
    void        run_();
    void        dataHandle_(size_t bytesAvailable);

    static constexpr char        TAG[]          = "CLI_TASK";
    static constexpr uart_port_t UART_PORT      = UART_NUM_0;
    static constexpr gpio_num_t  UART_TX        = GPIO_NUM_16;
    static constexpr gpio_num_t  UART_RX        = GPIO_NUM_17;
    static constexpr size_t      TX_BUFF_SIZE   = 0;
    static constexpr size_t      RX_BUFF_SIZE    = 256;

    Config          config_         = {};
    TaskHandle_t    taskHandle_     = nullptr;
    bool            initialized_    = false;

    uart_fd_t       uartFd_     = UART_FD_INVALID;
    Cli             cli_;
    QueueHandle_t   uartQueue_  = nullptr;
};

#endif // CLI_TASK_HPP
