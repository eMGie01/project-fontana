/**
 * @file app.hpp
 * @author Marek Gałeczka
 * @brief Application initialization and shutdown interface.
 * @version 0.1
 * @date 2026-04-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef APP_HPP
#define APP_HPP

enum class app_InitStatus
{
    ONGOING,
    RESTART,
    FATAL,
    DONE,
};

app_InitStatus app_Init();
// void app_Deinit();

#endif // APP_HPP
