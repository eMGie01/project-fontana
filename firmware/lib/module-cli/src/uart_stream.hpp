/**
 * @file uart_stream.hpp
 * @author Marek Gałeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef UART_STREAM_HPP
#define UART_STREAM_HPP

#include "iostream.hpp"
#include "uart.h"

class UartStream : public IoStream
{
public:
    explicit UartStream(uart_fd_t fd)
    : Fd_(fd)
    {}

    int 
    read(void * buf, size_t count)
    override
    {
        return uart_read(Fd_, buf, count);
    }

    int
    write(const void * buf, size_t count)
    override
    {
        return uart_write(Fd_, buf, count);
    }

private:
    uart_fd_t Fd_;

};

#endif // UART_STREAM_HPP