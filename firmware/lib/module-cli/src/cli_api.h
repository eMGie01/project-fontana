/**
 * @file cli_api.h
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-20
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLI_API_H
#define CLI_API_H

#include <stdint.h>
#include "esp_err.h"

typedef int (*cli_CommandHandler_t)(int argc, char** argv, char* response, size_t responseSize);

/**
 * @brief Structure containing parameters neccessarry for registrating commands
 */
typedef struct
{
    const char*             name;
    cli_CommandHandler_t    handler;
    const char*             help;
} cli_Command_t;

esp_err_t cli_RegisterCommand(const cli_Command_t* entry);

#endif // CLI_API_H