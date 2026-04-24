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

typedef enum
{
    INIT_ONGOING = 0,
    INIT_RESTART,
    INIT_FATAL,
    INIT_DONE
} init_status_t;

init_status_t app_init();
void app_deinit();

#endif // APP_HPP
