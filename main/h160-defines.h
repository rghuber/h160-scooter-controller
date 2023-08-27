#include <stdio.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"


#ifndef H160_DEFINES
#define H160_DEFINES

#define TAG_GPIO "GPIO"
#define TAG_I2C "I2C"
#define TAG_SPI "SPI"
#define TAG_MCPWM "MCPWM"
#define TAG_SDCARD "SDCARD"
#define TAG_NVS "NVS"
#define TAG_BLE "BLE"

#define I2C_SDA_PIN 7
#define I2C_SCL_PIN 6
#define I2C_MASTER_FREQ_HZ 100000

#define SPI_MOSI_PIN 11
#define SPI_MISO_PIN 13
#define SPI_SCLK_PIN 12
#define SPI_BUFFER_LEN 256
#define SPI_CS_IMU_PIN 10

#define SDMMC_CLK_PIN 2
#define SDMMC_CMD_PIN 1
#define SDMMC_D0_PIN 39
#define SDMMC_D1_PIN 40
#define SDMMC_D2_PIN 41
#define SDMMC_D3_PIN 42
#define SDMMC_DET_PIN 38

#define MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define MCPWM_FREQ_HZ             16384    // 16KHz PWM
#define MCPWM_DUTY_TICK_MAX       (MCPWM_TIMER_RESOLUTION_HZ / MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks
#define MCPWM_GPIO_A              35

#define BUZZER_PIN 48

#define TRIGGER_PIN 45
#define TRIGGER_SHORT_LONG_TICKS 40
#define DEBOUNCETIME 10


typedef struct {
    int16_t angrate_x;
    int16_t angrate_y;
    int16_t angrate_z;
    int16_t linacc_x;
    int16_t linacc_y;
    int16_t linacc_z;
} acc_gyr_t;

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} rtc_time_t;


typedef struct {
    int16_t adc0;
    int16_t adc1;
    int16_t adc2;
    int16_t adc3;
} adc_voltage_t;

#endif
