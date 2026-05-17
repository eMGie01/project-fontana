/**
 * @file taskMeas.cpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "taskMeas.hpp"

#include <stdint.h>
#include "esp_log.h"


static const char * tag = "MEAS_Task";

static QueueHandle_t* s_MeasEventQueue = nullptr;
static hx711_TypeDef* s_hx711;
static Meas* s_meas;

void IRAM_ATTR
meas_DataReadyCallback(void* arg)
{
    BaseType_t hpTaskWoken = pdFALSE;
    meas_TaskEventTypeDef event = {};
    event.type = MEAS_TASK_EVENT_HX711_READY;

    xQueueSendFromISR(*s_MeasEventQueue, &event, &hpTaskWoken);
    
    if (hpTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

bool meas_TaskSendCmd(const meas_TaskCmdTypeDef* cmd)
{
	if (s_MeasEventQueue == NULL || *s_MeasEventQueue == NULL || cmd == NULL)
	{
		return false;
	}
	
	meas_TaskEventTypeDef event = {};
	event.type = MEAS_TASK_EVENT_CMD;
	event.data.cmd = *cmd;
	return (xQueueSend(*s_MeasEventQueue, &event, portMAX_DELAY) == pdTRUE);
}

static meas_StatusTypeDef
MEAS_TaskHandleCmd(const meas_TaskCmdTypeDef* cmd)
{
	if (cmd == nullptr)
	{
		return MEAS_ERR_INVAL;
	}

	switch (cmd->type)
	{
	case MEAS_TASK_CMD_RESET:
		return s_meas->Ioctl(MEAS_IOCTL_RESET, nullptr);

	case MEAS_TASK_CMD_SET_OFFSET:
	{
		int32_t value = cmd->arg.codeOffset;
		return s_meas->Ioctl(MEAS_IOCTL_SET_CODE_OFFSET, &value);
	}

	case MEAS_TASK_CMD_SET_COUNTS_PER_UMHG:
	{
		int32_t value = cmd->arg.codeCountsPerUmHg;
		return s_meas->Ioctl(MEAS_IOCTL_SET_CODE_COUNTS_PER_UMHG, &value);
	}

	case MEAS_TASK_CMD_SET_IIR_SHIFT:
	{
		uint8_t value = cmd->arg.iirShift;
		return s_meas->Ioctl(MEAS_IOCTL_SET_IIR_SHIFT, &value);
	}

	case MEAS_TASK_CMD_SET_AVG_WINDOW_SIZE:
	{
		uint8_t value = cmd->arg.avgWindowSize;
		return s_meas->Ioctl(MEAS_IOCTL_SET_AVG_WINDOW_SIZE, &value);
	}

	default:
		return MEAS_ERR_INVAL;
	}
}

static bool
MEAS_TaskTaskReadADC()
{
	int32_t code;
	meas_ReadTypeDef readStruct;

	hx711_StatusTypeDef hx711St = hx711_Read(s_hx711, &code);
	if (hx711St != HX711_ERR_OK)
	{
		ESP_LOGW(tag, "hx711_Read failed with error (%d)", hx711St);
		return false;
	}
	s_meas->Write(code);
	meas_StatusTypeDef measSt = s_meas->Read(readStruct);
	if (measSt != MEAS_ERR_OK)
	{
		ESP_LOGE(tag, "performing calculations in Meas Class failed with error (%d)", measSt);
		return false;
	}
	
	uint64_t time = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
	ESP_LOGI(tag, "[%lld, %ld, %lld, %lld]", time, code, readStruct.umHgAvg, readStruct.umHgFilt);
	return true;
}

void 
meas_Task(void* pvParameters)
{
	meas_TaskContextTypeDef* ctx = static_cast<meas_TaskContextTypeDef*>(pvParameters);
	if (ctx == nullptr || ctx->hx711 == nullptr || ctx->eventQueue == nullptr || *ctx->eventQueue == nullptr || ctx->meas == nullptr)
	{
		ESP_LOGE(tag, "measTask init failed");
		vTaskDelete(nullptr); // albo esp_restart(), jeśli to błąd krytyczny
		return;
	}

	s_hx711 = ctx->hx711;
	s_MeasEventQueue = ctx->eventQueue;
	s_meas = ctx->meas;
	
	meas_TaskEventTypeDef event;
	for (;;)
	{
		if (xQueueReceive(*s_MeasEventQueue, &event, portMAX_DELAY) == pdTRUE)
		{
			if (event.type == MEAS_TASK_EVENT_HX711_READY)
			{
				if (MEAS_TaskTaskReadADC() == true)
				{
					// save to snapshot;
				}
			}
			else if (event.type == MEAS_TASK_EVENT_CMD)
			{
				meas_StatusTypeDef st = MEAS_TaskHandleCmd(&event.data.cmd);
				if (st != MEAS_ERR_OK)
				{
					ESP_LOGW(tag, "meas command failed: %d", st);
				}
			}
			else
			{
				ESP_LOGE(tag, "event type unexpectedfailure");
				continue;
			}
		}
	}
}
