#pragma once

#include "cli_api2.hpp"
#include "meas_task.hpp"

class CliMeasCmdEntry : public CliCommandEntry
{
public:
    explicit CliMeasCmdEntry(MeasControlApi* measApi, CliControlApi* cliApi)
    : measApi_(measApi)
    , cliApi_(cliApi)
    {}

    ErrStatus execute(const char* subcommand, int argc, char** argv, char* response, size_t resSize) override;
    ErrStatus cmdRegister() override;

private:
    MeasControlApi* measApi_;
    CliControlApi* cliApi_;

    ErrStatus cmdMeas_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetOffset_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetCountsPerUmHg_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetIirShift_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetAvgWindow_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdReset_(int argc, char** argv, char* resp, size_t size);
};
