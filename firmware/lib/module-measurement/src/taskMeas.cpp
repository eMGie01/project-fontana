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


static const char * tag = "MEAS_TASK";

static QueueHandle_t* s_MeasEventQueue = nullptr;
static hx711_TypeDef* s_hx711;
static Meas* s_meas;

void IRAM_ATTR
MEAS_Hx711DataReadyCallback(void* arg)
{
    BaseType_t hpTaskWoken = pdFALSE;
    MEAS_TaskEvent event = {};
    event.type = MEAS_TASK_EVENT_HX711_READY;

    xQueueSendFromISR(*s_MeasEventQueue, &event, &hpTaskWoken);
    
    if (hpTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

bool MEAS_TaskSendCmd(const MEAS_TaskCmd* cmd)
{
	if (s_MeasEventQueue == NULL || cmd == NULL)
	{
		return false;
	}
	
	MEAS_TaskEvent event = {};
	event.type = MEAS_TASK_EVENT_CMD;
	event.data.cmd = *cmd;
	return (xQueueSend(*s_MeasEventQueue, &event, portMAX_DELAY) == pdTRUE);
}

static void
MEAS_TaskReadADC()
{
	int32_t code;
	MEAS_ReadStructDef readStruct;

	hx711_Read(s_hx711, &code);
	s_meas->Write(code);
	s_meas->Read(readStruct);
	
	uint64_t time = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

	ESP_LOGI(tag, "[%lld, %ld, %lld, %lld]", time, code, readStruct.umHgAvg, readStruct.umHgFilt);
}

void 
measTask(void* pvParameters)
{
	MEAS_TaskContext* ctx = (MEAS_TaskContext*)pvParameters;

	s_hx711 = ctx->hx711;
	s_MeasEventQueue = ctx->eventQueue;
	s_meas = ctx->meas;
	
	MEAS_TaskEvent event;
	int32_t code;
	for (;;)
	{
		if (xQueueReceive(*s_MeasEventQueue, &event, portMAX_DELAY) == pdTRUE)
		{
			if (event.type == MEAS_TASK_EVENT_HX711_READY)
			{
				MEAS_TaskReadADC();
			}
			else if (event.type == MEAS_TASK_EVENT_CMD)
			{
				// switch statement for commands :3
			}
			else
			{
				// log smth
				continue;
			}
		}
	}
}