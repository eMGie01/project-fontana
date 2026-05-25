/**
 * @file err_status.hpp
 * @author Marek Galeczka (marek.galeczka@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef ERR_STATUS_HPP
#define ERR_STATUS_HPP

enum class ErrStatus
{
    FAIL,
    NODEV,
    INVAL,
    TIMEOUT,
    RUNTIME,
    OK,
};

#endif // ERR_STATUS_HPP