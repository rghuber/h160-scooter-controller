#include "h160-defines.h"

extern adc_voltage_t adc_voltages;

#define ADS1115_CONV_R 0b00000000
#define ADS1115_CONF_R 0b00000001
#define ADS1115_LOTH_R 0b00000010
#define ADS1115_HITH_R 0b00000011

#define ADS1115_I2C_ADDR 0x48

esp_err_t DEVICE_ADC_init();
esp_err_t DEVICE_ADC_read_registers();
esp_err_t DEVICE_ADC_read_voltages(adc_voltage_t *adc_voltages);
void DEVICE_ADC_print_voltages(adc_voltage_t *adc_voltages);