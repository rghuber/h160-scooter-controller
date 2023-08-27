#include "driver/gpio.h"
#include "h160-defines.h"

// GPIO DEVICES

esp_err_t PERIPH_gpio_init()
{
    gpio_set_direction(MCPWM_GPIO_A, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(MCPWM_GPIO_A, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_mode(MCPWM_GPIO_A, GPIO_PULLUP_DISABLE);

    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(BUZZER_PIN, GPIO_PULLDOWN_ONLY);

    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(TRIGGER_PIN, GPIO_PULLDOWN_DISABLE);
    gpio_set_pull_mode(TRIGGER_PIN, GPIO_PULLUP_DISABLE);
    gpio_set_intr_type(TRIGGER_PIN, GPIO_INTR_ANYEDGE);

    gpio_set_pull_mode(SPI_MISO_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_MOSI_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_SCLK_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_CS_IMU_PIN, GPIO_PULLUP_ONLY);

    gpio_set_pull_mode(SDMMC_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDMMC_CLK_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDMMC_D0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDMMC_D1_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDMMC_D2_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SDMMC_D3_PIN, GPIO_PULLUP_ONLY);

    return ESP_OK;
}