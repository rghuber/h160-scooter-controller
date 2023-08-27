#include <string.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "h160-adc.h"
#include "h160-defines.h"

adc_voltage_t adc_voltages;

esp_err_t DEVICE_ADC_read_registers(){
    printf("ADC REGISTER CONTENTS:\n");
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    esp_err_t ret;
    
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_CONF_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_CONV_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    txbuf[0] = ADS1115_LOTH_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_LOTH_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    txbuf[0] = ADS1115_HITH_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_HITH_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    return ret;
}

esp_err_t DEVICE_ADC_read_voltages(adc_voltage_t *adc_voltages){
    uint8_t txbuf[2];
    uint8_t rxbuf[2];
    esp_err_t ret;

    // read channel 0
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);

    txbuf[1] = 0b11000011;
    txbuf[0] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,2,portMAX_DELAY);

    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    memcpy(&adc_voltages->adc0,rxbuf,2);

    // read channel 1
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);

    txbuf[1] = 0b11010011;
    txbuf[0] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,2,portMAX_DELAY);

    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    memcpy(&adc_voltages->adc1,rxbuf,2);

    // read channel 2
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);

    txbuf[1] = 0b11100011;
    txbuf[0] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,2,portMAX_DELAY);

    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    memcpy(&adc_voltages->adc2,rxbuf,2);

    // read channel 3
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);

    txbuf[1] = 0b11110011;
    txbuf[0] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,2,portMAX_DELAY);

    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,0x08,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x08,rxbuf,2,portMAX_DELAY);
    memcpy(&adc_voltages->adc3,rxbuf,2);

    return ret;
}
