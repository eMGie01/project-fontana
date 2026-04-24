# project-fontana

## Firmware file tree

```
firmware
├── CMakeLists.txt
├── include
│   ├── app.hpp
│   └── task_structures.hpp
├── lib
│   ├── cli
│   │   └── src
│   │       ├── cli_handles.hpp
│   │       ├── cli_handles_hx711.cpp
│   │       ├── cli_handles_meas.cpp
│   │       ├── cli_interface.cpp
│   │       └── cli_interface.hpp
│   ├── hx711-esrpessif-driver
│   │   ├── README.md
│   │   └── src
│   │       ├── hx711.h
│   │       ├── hx711_init.c
│   │       └── hx711_meas.c
│   ├── lcd
│   │   └── src
│   │       ├── lcd.c
│   │       └── lcd.h
│   ├── measurement
│   │   └── src
│   │       ├── measurement.cpp
│   │       └── measurement.hpp
│   ├── snapshot
│   │   └── src
│   │       ├── snapshot.c
│   │       └── snapshot.h
│   └── uart_utils
│       └── src
│           ├── my_uart.c
│           ├── my_uart.h
│           └── uart_default.c
├── platformio.ini
├── src
│   ├── app_init.cpp
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── task_cli.cpp
│   ├── task_lcd.cpp
│   └── task_meas.cpp
└── test
    └── README
```
