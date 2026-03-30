#include "console_io.hpp"

#include <cstdio>
#include <cstring>

bool ConsoleIO::init()
{
    // Dla USB Serial/JTAG console zwykle nic nie trzeba inicjalizować.
    // Konsola jest stawiana przez ESP-IDF.
    return true;
}

bool ConsoleIO::readLine(char *buffer, std::size_t buffer_size)
{
    if (!buffer || buffer_size < 2)
    {
        return false;
    }

    std::memset(buffer, 0, buffer_size);

    // Blokujący odczyt jednej linii ze stdin.
    // Działa z USB console po ustawieniu jej w menuconfig.
    char *res = std::fgets(buffer, static_cast<int>(buffer_size), stdin);
    if (!res)
    {
        return false;
    }

    // Usuń końcowe \r i \n
    std::size_t len = std::strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
    {
        buffer[len - 1] = '\0';
        --len;
    }

    return true;
}

void ConsoleIO::write(const char *text)
{
    if (!text)
    {
        return;
    }

    std::fputs(text, stdout);
    std::fflush(stdout);
}

void ConsoleIO::writeln(const char *text)
{
    if (text)
    {
        std::fputs(text, stdout);
    }
    std::fputs("\r\n", stdout);
    std::fflush(stdout);
}