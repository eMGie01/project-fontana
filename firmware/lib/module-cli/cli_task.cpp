#include "cli_task.hpp"

#include <cstring>

// 

ErrStatus CliTask::
init(Config cfg)
{
    if (initialized_ == true)
    {
        ESP_LOGE(TAG, "cannot initialize task, task already initialized");
        return ErrStatus::FAIL;
    }
    config_ = cfg;

    uart_open_cfg_t uartCfg = {};
    uartCfg.port = UART_PORT;
    uartCfg.io_tx = UART_TX;
    uartCfg.io_rx = UART_RX;
    uartCfg.baud = 115200;
    uartCfg.tx_buf_size = TX_BUFF_SIZE;
    uartCfg.rx_buf_size = RX_BUFF_SZE;

    // Initializing connection with UART
    uartFd_ = uart_open(&uartCfg);
    if (uartFd_ == UART_FD_INVALID)
    {
        ESP_LOGE(TAG, "initializing connection with UART failed");
        return ErrStatus::FAIL;
    }

    // get access to UARTs EventQueue
    if (uart_ioctl(uartFd_, UART_IOCTL_GET_EVQUEUE, &uartQueue_) != 0)
    {
        ESP_LOGE(TAG, "getting access to uart's event Queue failed");
        uart_close(uartFd_);
        uartFd_ = UART_FD_INVALID;
        return ErrStatus::FAIL;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "task initialized successfully");
    return ErrStatus::OK;
}

ErrStatus CliTask::
start()
{
    if (taskHandle_ != nullptr)
    {
        ESP_LOGE(TAG, "start failed, task handle is not a nullptr");
        return ErrStatus::FAIL;
    }

    BaseType_t taskRes = xTaskCreate(taskEntry_, TAG, config_.stackSize, this, config_.priority, &taskHandle_);
    if (taskRes != pdTRUE)
    {
        ESP_LOGE(TAG, "task creation failed");
        return ErrStatus::FAIL;
    }

    ESP_LOGI(TAG, "task started successfully");
    return ErrStatus::OK;
}

ErrStatus CliTask::
stop()
{
    if (taskHandle_ == nullptr)
    {
        ESP_LOGE(TAG, "cannot stop task, not running");
        return ErrStatus::OK;
    }

    vTaskDelete(taskHandle_);
    taskHandle_ = nullptr;

    if (uartQueue_ != nullptr)
    {
        if (xQueueReset(uartQueue_) == pdFALSE)
        {
            ESP_LOGE(TAG, "reset of uart Queue failed");
            return ErrStatus::FAIL;
        }
    }

    initialized_ = false;
    ESP_LOGI(TAG, "task stopped successfully");
    return ErrStatus::OK;
}

ErrStatus CliTask::
registerCommand(const Command& entry)
{
    if (taskHandle_ != nullptr)
    {
        ESP_LOGE(TAG, "cannot register command after task start");
        return ErrStatus::FAIL;
    }
    return cli_.registerCmd(entry);
}

// 

void CliTask::
taskEntry_(void* pvParameters)
{
    auto* self = static_cast<CliTask*>(pvParameters);
    self->run_();
}

void CliTask::
run_()
{
    uart_event_t uartEvent;
    for (;;)
    {
        if (xQueueReceive(uartQueue_, &uartEvent, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGD(TAG, "uart event type=%d size=%d", uartEvent.type, uartEvent.size);
            if (uartEvent.type != UART_DATA)
            {
                uart_ioctl(uartFd_, UART_IOCTL_FLUSH_RX, nullptr);
                xQueueReset(uartQueue_);
                continue;
            }
            dataHandle_(uartEvent.size);
        }
    }
}

void CliTask::
dataHandle_(size_t bytesAvailable)
{
    char rxBuf[64];
    char response[128];
    while (bytesAvailable > 0)
    {
        size_t bytesToRead = (bytesAvailable < sizeof(rxBuf)) ? bytesAvailable : sizeof(rxBuf);
        int bytesRead = uart_read(uartFd_, rxBuf, bytesToRead);
        ESP_LOGD(TAG, "uart_read=%d", bytesRead);
        if (bytesRead <= 0)
        {
            return;
        }

        bytesAvailable -= bytesRead;
        for (int i = 0; i < bytesRead; ++i)
        {
            cli_.push(rxBuf[i]);

            if (!cli_.hasLine())
            {
                continue;
            }

            response[0] = '\0';
            // if (cli_.execute(response, sizeof(response)) == ESP_OK && response[0] != '\0')
            // {
            //     uart_write(s_UartFd, response, std::strlen(response));
            // }
            ErrStatus st = cli_.execute(response, sizeof(response));
            // ESP_LOGD(TAG, "st=%d response='%s'", st, response);
            if (st == ErrStatus::OK && response[0] != '\0')
            {
                int written = uart_write(uartFd_, response, std::strlen(response));
                if (written <= 0)
                {
                    continue;
                }
                ESP_LOGD(TAG, "%s", response);
            }
        }
    } 
}

// 
