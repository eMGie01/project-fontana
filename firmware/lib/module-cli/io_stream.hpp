/**
 * @file io_stream.hpp
 * @author Marek Gałeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef IO_STREAM_HPP
#define IO_STREAM_HPP

#include <stddef.h>

class IoStream
{
public:
    virtual ~IoStream() = default;
    virtual int read(void * buf, size_t count) = 0;
    virtual int write(const void * buf, size_t count) = 0;
};

#endif // IOSTREAM_HPP