#pragma once

#include <cstddef>

class ConsoleIO
{
public:
    ConsoleIO() = default;

    bool init();
    bool readLine(char *buffer, std::size_t buffer_size);
    void write(const char *text);
    void writeln(const char *text);
};