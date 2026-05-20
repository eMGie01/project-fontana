/**
 * @file command_line_interface.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef CLI_HPP
#define CLI_HPP

#include "cli_api.h"

#include <stddef.h>
#include "esp_err.h"

/**
 * @brief Class of Command-Line Interface
 */
class Cli
{
public:
    static constexpr size_t MAX_COMMANDS    = 32;
    static constexpr size_t LINE_BUF_SIZE   = 128;
    static constexpr size_t K_MAX_TOKENS    = 8;

    esp_err_t RegisterCommand(const cli_Command_t* entry);
    void Push(char c);
    bool HasLine() const;
    esp_err_t Execute(char* response, size_t size);

private:
    const char* tag = "CLI";
    
    size_t tokenizeLine_(char** tokens);
    void dispatchCommand_(char** tokens, size_t count, char* response, size_t responseSize);

    char line_[LINE_BUF_SIZE] = {};
    size_t pos_ = 0;
    bool lineReady_ = false;

    cli_Command_t commands_[MAX_COMMANDS] = {};
    size_t commandCount_ = 0;
};

#endif // CLI_HPP
