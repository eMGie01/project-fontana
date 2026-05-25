#ifndef CLI_MEAS_HANDLER_HPP
#define CLI_MEAS_HANDLER_HPP

#include "cli_api.hpp"
#include "err_status.hpp"

#include <stddef.h>

class MeasControlApi;

class CliMeasCmdEntry : public CliCommandEntry
{
public:
    CliMeasCmdEntry(CliControlApi* cliApi, MeasControlApi* measApi)
    : CliCommandEntry(cliApi)
    , measApi_(measApi)
    {}

    ErrStatus cmdRegister() override;

private:
    static ErrStatus cmdMeasThunk_(void* context, int argc, char** argv, char* resp, size_t size);
    static ErrStatus cmdSetOffsetThunk_(void* context, int argc, char** argv, char* resp, size_t size);
    static ErrStatus cmdSetCountsPerUmHgThunk_(void* context, int argc, char** argv, char* resp, size_t size);
    static ErrStatus cmdSetIirShiftThunk_(void* context, int argc, char** argv, char* resp, size_t size);
    static ErrStatus cmdSetAvgWindowThunk_(void* context, int argc, char** argv, char* resp, size_t size);
    static ErrStatus cmdResetThunk_(void* context, int argc, char** argv, char* resp, size_t size);

    ErrStatus cmdMeas_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetOffset_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetCountsPerUmHg_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetIirShift_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdSetAvgWindow_(int argc, char** argv, char* resp, size_t size);
    ErrStatus cmdReset_(int argc, char** argv, char* resp, size_t size);

    MeasControlApi* measApi_;
};

#endif // CLI_MEAS_HANDLER_HPP
