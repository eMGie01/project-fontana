# Project Fontana Firmware

Firmware for an ESP32-C6 based pressure measurement and SD logging device.

The application reads raw samples from an HX711 ADC, converts and filters the
measurement, displays the current state on an ST7789 LCD, exposes calibration
commands over UART CLI, stores measurement settings in NVS, and logs CSV data to
an SD card.

## Hardware Target

- Board: ESP32-C6-DevKitM-1
- Framework: ESP-IDF through PlatformIO
- Display: ST7789 LCD on SPI2
- ADC: HX711
- Storage: SD card over SDSPI
- Control: active-low button with internal pull-up
- CLI: UART0, 115200 baud

Pin assignments are defined in `include/hw_config.h`.

## Build

```sh
/home/majg__/.platformio/penv/bin/pio run
```

Or, if PlatformIO is available in `PATH`:

```sh
pio run
```

The active PlatformIO environment is:

```ini
[env:esp32-c6-devkitm-1]
platform = espressif32
board = esp32-c6-devkitm-1
framework = espidf
monitor_speed = 115200
```

## Upload And Monitor

```sh
pio run -t upload
pio device monitor
```

If `pio` is not in `PATH`, use the full PlatformIO path shown in the build
section.

## Runtime Behavior

On startup the firmware:

1. Initializes NVS.
2. Initializes GPIO ISR service and SPI2.
3. Mounts the SD card at `/sdcard`.
4. Initializes the snapshot, CLI, measurement, UI, and SD logger tasks.
5. Loads persisted measurement settings from NVS.
6. Starts the FreeRTOS tasks.

The SD logger creates CSV files in:

```text
/sdcard/data/dat_XXXX.csv
```

File names are FAT 8.3 compatible. Logging is buffered in RAM and flushed to the
SD card every 100 samples or when recording is disabled.

## UI

The LCD shows:

- filtered pressure value,
- averaged value when available,
- measurement offset and scale,
- averaging and IIR filter settings,
- SD recording state: `SD:OFF` or `SD:REC`.

The button toggles SD recording with a short press. Very short presses are
ignored as debounce noise.

## CLI

The CLI command group is `meas`.

```text
meas help
meas reset
meas set_offset <counts>
meas set_counts <counts_per_umhg>
meas set_iir <shift>
meas set_avgwin <size>
```

Settings changed through CLI are persisted in NVS:

- offset,
- counts per umHg,
- IIR shift,
- average window size.

`meas reset` resets runtime measurement state only; it does not erase saved NVS
configuration.

## Project Layout

```text
include/                    Shared project headers and hardware configuration
src/                        Application entry point and system initialization
lib/driver-button/          Active-low GPIO button wrapper
lib/driver-hx711/           HX711 raw ADC driver
lib/driver-lcd/             ST7789 LCD wrapper
lib/driver-sd/              SD card mount and file writer
lib/driver-uart/            UART wrapper used by CLI
lib/module-cli/             Command parser and measurement CLI commands
lib/module-measurement/     Measurement task, filtering, averaging, calibration
lib/module-sd-logger/       Buffered CSV logger task
lib/module-snapshot/        Thread-safe shared measurement snapshot
lib/module-ui/              LCD UI and button handling task
```

## Data Format

CSV header:

```csv
time_ms,code,filtered,averaged,offset,counts,iirShift,avgWin
```

Rows contain the current measurement snapshot at the time the sample is handed to
the SD logger.

## Notes

- SD card access is expected during application startup. If mounting fails, the
  app init path returns restart status.
- Long file names are not required for logger output; generated file names stay
  within FAT 8.3 limits.
- Hardware-specific constants should stay centralized in `include/hw_config.h`.
