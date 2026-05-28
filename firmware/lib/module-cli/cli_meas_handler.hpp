/**
 * @file cli_meas_handler.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "cli_api.hpp"
#include "meas_task.hpp"

class CliMeasCmdEntry : public CliCommandEntry
{
public:
    explicit CliMeasCmdEntry(MeasControlApi& measApi, CliControlApi& cliApi)
    : measApi_(measApi)
    , cliApi_(cliApi)
    {}

    ErrStatus execute(const char* subcommand, int argc, char** argv, char* response, size_t resSize) override;
    ErrStatus cmdRegister() override;

private:
    MeasControlApi& measApi_;
    CliControlApi& cliApi_;

    ErrStatus cmdMeas_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetOffset_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetCountsPerUmHg_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetIirShift_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetAvgWindow_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdReset_(int argc, char** argv, char* resp, size_t size);
};
