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

#include "err_status.hpp"
#include "cli_api.hpp"

#include <cstdio>

/**
 * @brief Class of Command-Line Interface
 */
class Cli
{
public:

    static constexpr size_t MAX_COMMANDS    = 32;
    static constexpr size_t LINE_BUF_SIZE   = 128;
    static constexpr size_t K_MAX_TOKENS    = 8;

    ErrStatus   registerCmd(const CliControlApi::Command& entry);
    void        push(char c);
    bool        hasLine() const;
    ErrStatus   execute(char* response, size_t size);

private:

    static constexpr char TAG[] = "CLI";
    
    size_t      tokenizeLine_(char** tokens);
    ErrStatus   dispatchCommand_(char** tokens, size_t count, char* response, size_t responseSize);

    char    line_[LINE_BUF_SIZE] = {};
    size_t  pos_ = 0;
    bool    lineReady_ = false;

    CliControlApi::Command  commands_[MAX_COMMANDS] = {};
    size_t                  commandCount_ = 0;
};

#endif // CLI_HPP
