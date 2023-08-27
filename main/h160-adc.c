#include <string.h>
#include <stdlib.h>
#include "driver/i2c.h"
#include "h160-adc.h"
#include "h160-defines.h"

adc_voltage_t adc_voltages;

esp_err_t DEVICE_ADC_init(){
    uint8_t txbuf[3];
    esp_err_t ret;

    txbuf[0] = ADS1115_CONF_R;
    txbuf[1] = 0b11000010;
    txbuf[2] = 0b10000011;
    ret=i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,3,portMAX_DELAY);
    return ret;
}    

esp_err_t DEVICE_ADC_read_registers(){
    printf("ADC REGISTER CONTENTS:\n");
    uint8_t txbuf[2];
    uint8_t rxbuf[2];

    esp_err_t ret;
    
    txbuf[0] = ADS1115_CONF_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_CONF_R: %02x%02x\n",rxbuf[0],rxbuf[1]);

    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_CONV_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    txbuf[0] = ADS1115_LOTH_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_LOTH_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    txbuf[0] = ADS1115_HITH_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    printf("ADS1115_HITH_R: %02x%02x\n",rxbuf[0],rxbuf[1]);
    
    return ret;
}

esp_err_t DEVICE_ADC_read_voltages(adc_voltage_t *adc_voltages){
    uint8_t txbuf[3];
    uint8_t rxbuf[2];
    uint8_t vbuf[2];
    esp_err_t ret;

    txbuf[0] = ADS1115_CONF_R;
    txbuf[1] = 0b11000010;
    txbuf[2] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,3,portMAX_DELAY);
    vTaskDelay(50/portTICK_PERIOD_MS);
    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    vbuf[0]=rxbuf[1];
    vbuf[1]=rxbuf[0];
    memcpy(&adc_voltages->adc0,vbuf,2);
    
    if(ret != ESP_OK){
        ESP_LOGE(TAG_ADC,"error reading from ADC");
    }

    // read channel 1
    txbuf[0] = ADS1115_CONF_R;
    txbuf[1] = 0b11010010;
    txbuf[2] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,3,portMAX_DELAY);
    vTaskDelay(50/portTICK_PERIOD_MS);
    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    vbuf[0]=rxbuf[1];
    vbuf[1]=rxbuf[0];
    memcpy(&adc_voltages->adc1,vbuf,2);

    // read channel 2
    txbuf[0] = ADS1115_CONF_R;
    txbuf[1] = 0b11100010;
    txbuf[2] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,3,portMAX_DELAY);
    vTaskDelay(50/portTICK_PERIOD_MS);
    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    vbuf[0]=rxbuf[1];
    vbuf[1]=rxbuf[0];
    memcpy(&adc_voltages->adc2,vbuf,2);

    // read channel 3
    txbuf[0] = ADS1115_CONF_R;
    txbuf[1] = 0b11110010;
    txbuf[2] = 0b10000011;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,3,portMAX_DELAY);
    vTaskDelay(50/portTICK_PERIOD_MS);
    txbuf[0] = ADS1115_CONV_R;
    i2c_master_write_to_device(I2C_NUM_0,ADS1115_I2C_ADDR,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,ADS1115_I2C_ADDR,rxbuf,2,portMAX_DELAY);
    vbuf[0]=rxbuf[1];
    vbuf[1]=rxbuf[0];
    memcpy(&adc_voltages->adc3,vbuf,2);

    return ret;
}

void DEVICE_ADC_print_voltages(adc_voltage_t *adc_voltages){
    DEVICE_ADC_read_voltages(adc_voltages);
    float fs=4.096;
    printf("ADC Channel 0: %.3fV\n",((float)adc_voltages->adc0)/32768.0*fs);
    printf("ADC Channel 1: %.3fV\n",((float)adc_voltages->adc1)/32768.0*fs);
    printf("ADC Channel 2: %.3fV\n",((float)adc_voltages->adc2)/32768.0*fs);
    printf("ADC Channel 3: %.3fV\n",((float)adc_voltages->adc3)/32768.0*fs);
    printf("\n");
}