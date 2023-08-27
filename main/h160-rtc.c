#include "driver/i2c.h"
#include "h160-defines.h"

rtc_time_t rtc_date_time;

esp_err_t DEVICE_RTC_read_registers(){
    printf("RTC REGISTER CONTENTS:\n");
    int i;
    uint8_t txbuf[1] = {0x00};
    uint8_t rxbuf[19];
    esp_err_t ret;
    i2c_master_write_to_device(I2C_NUM_0,0x68,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x68,rxbuf,19,portMAX_DELAY);
    for(i=0;i<19;i++){
       if(i%16 == 0){
            printf("0x%02x",i);
        }
        if(i%2==0){
            printf(" ");
        }
        printf("%02x",rxbuf[i]);
        if(i%16 == 15){
            printf("\n");
        }
    }
    printf("\n");
    return ret;
}

esp_err_t DEVICE_RTC_read_rtctime(rtc_time_t *rtc_date_time){
    uint8_t txbuf[1] = {0x00};
    uint8_t rxbuf[7];
    esp_err_t ret;
    i2c_master_write_to_device(I2C_NUM_0,0x68,txbuf,1,portMAX_DELAY);
    ret=i2c_master_read_from_device(I2C_NUM_0,0x68,rxbuf,7,portMAX_DELAY);
    
    // decoding for DS3231M RTC chip
    // seconds 4LSB is sec, 4MSB is 10sec
    rtc_date_time->sec=(rxbuf[0]&0b00001111)+10*(rxbuf[0]>>4);
    // minutes 4LSB is sec, 4MSB is 10sec
    rtc_date_time->min=(rxbuf[1]&0b00001111)+10*(rxbuf[1]>>4);
    // hour 4LSB is day 0-9, 4MSB is 10sec
    rtc_date_time->min=(rxbuf[2]&0b00001111)+10*((rxbuf[2]&0b00010000)>>4)+20*((rxbuf[2]&0b00100000)>>5);
    // weekday is trivial
    rtc_date_time->weekday=rxbuf[3];
    // day 4LSB is day 0-9, 4MSB is 10day
    rtc_date_time->day=(rxbuf[4]&0b00001111)+10*((rxbuf[4]&0b00110000)>>4);
    // month 4LSB is month 0-9, 4MSB is 10month
    rtc_date_time->month=(rxbuf[5]&0b00001111)+10*((rxbuf[5]&0b00010000)>>4);
    // year 4LSB is year 0-9, 4MSB is 10year
    rtc_date_time->year=(rxbuf[6]&0b00001111)+10*((rxbuf[6]&0b11110000)>>4);


    return ret;
}
