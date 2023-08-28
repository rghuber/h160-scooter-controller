// setup i2c master
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "h160-defines.h"

#include "h160-gpio.h"

#include "h160-spi.h"
#include "h160-imu.h"

#include "h160-sdcard.h"

#include "h160-i2c.h"
#include "h160-rtc.h"
#include "h160-adc.h"

#include "h160-mcpwm.h"

#include "h160-bluetooth.h"

#include "h160-ISR.h"

struct tm tm;
struct timeval now;
time_t epoch;
char timestamp[100];

void TASK_DataLogger(){

    FILE * data_log_f;
    char fname[100];
    sprintf(fname,"/sdcard/data_log_20%02d-%02d-%02dT%02d_%02d_%02d.dat",
                                                            rtc_date_time.year,
                                                            rtc_date_time.month,
                                                            rtc_date_time.day,
                                                            rtc_date_time.hour,
                                                            rtc_date_time.min,
                                                            rtc_date_time.sec
                                                            );
    data_log_f = fopen(fname, "wb");
    fclose(data_log_f);

    while(1){
        
        DEVICE_IMU_read_acc_gyr(imu,&acc_gyr);
        DEVICE_ADC_read_voltages(&adc_voltages);
        gettimeofday(&now,NULL);

        data_log_f = fopen(fname, "ab");
        fwrite(&now,sizeof(long long int)+sizeof(long int),1,data_log_f);
        fwrite(&acc_gyr,sizeof(acc_gyr_t),1,data_log_f);
        fwrite(&adc_voltages,sizeof(adc_voltage_t),1,data_log_f);
        fclose(data_log_f);
        
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void TASK_TriggerRead( void * parameter)
{

    TickType_t saveDebounceTimeout;
    bool saveLastState;
    int save;

    TickType_t timePressed = 10000000 ,timeReleased = 0; 
    bool isPressed = false;

    // Enter RTOS Task Loop
    while (1) {

        portENTER_CRITICAL_ISR(&mux); // so that value of numberOfButtonInterrupts,l astState are atomic - Critical Section
        save  = numberOfButtonInterrupts;
        saveDebounceTimeout = debounceTimeout;
        saveLastState  = lastState;
        portEXIT_CRITICAL_ISR(&mux); // end of Critical Section

        bool currentState = gpio_get_level(TRIGGER_PIN);

        // This is the critical IF statement
        // if Interrupt Has triggered AND Button Pin is in same state AND the debounce time has expired THEN you have the button push!
        
        if ((save != 0) //interrupt has triggered
            && (currentState == saveLastState) // pin is still in the same state as when intr triggered
            && (xTaskGetTickCount() - saveDebounceTimeout > DEBOUNCETIME ))
        { // and it has been low for at least DEBOUNCETIME, then valid keypress
            if (currentState == 0)
            {
                //printf("Button is released and debounced, current tick=%ld\n", xTaskGetTickCount());
                // note down time
                timeReleased = xTaskGetTickCount();
                if(isPressed){
                    STATE_change_speed();
                    printf("Short press, changing mode... ");
                    printf("Current motor speed setting: %d%%\n",MOTOR_speed);
                    isPressed = false;
                } else {
                    printf("We released the trigger at %ld, stopping...\n",timeReleased);
                    DEVICE_MCPWM_stop();
                }
            }
            else
            {
                //printf("Button is pressed and debounced, current tick=%ld\n", xTaskGetTickCount());
                // note down time
                timePressed = xTaskGetTickCount();
                isPressed = true;
            }
      
            //printf("Button Interrupt Triggered %d times, current State=%u, time since last trigger %ldms\n", save, currentState, (xTaskGetTickCount() - saveDebounceTimeout)*4);
      
            portENTER_CRITICAL_ISR(&mux); // can't change it unless, atomic - Critical section
            numberOfButtonInterrupts = 0; // acknowledge keypress and reset interrupt counter
            portEXIT_CRITICAL_ISR(&mux);

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        if(isPressed && xTaskGetTickCount()-timePressed > TRIGGER_SHORT_LONG_TICKS){
            printf("Long press, at tick %ld we are going...\n",xTaskGetTickCount());
            DEVICE_MCPWM_run();
            isPressed = false;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}


void app_main()
{
    esp_err_t ret;

    ESP_LOGI(TAG_NVS,"initializing nonvolatile storage...");
    ret=nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI(TAG_NVS,"erased NVS...");
        ret = nvs_flash_init();
        {
            ESP_LOGE(TAG_NVS,"NVS init failed");
        }
    }

    ESP_LOGI(TAG_GPIO,"initializing BLE..."); 
    startBLE();

    ESP_LOGI(TAG_GPIO,"initializing GPIO pins...");
    if (PERIPH_gpio_init() != ESP_OK)
    {
        ESP_LOGE(TAG_GPIO,"spi bus init failed");
    }
    
    ESP_LOGI(TAG_I2C,"initializing i2c");
    if (PERIPH_i2c_master_init() != ESP_OK){
        ESP_LOGE(TAG_I2C, "i2c init failed");
    }
    DEVICE_i2c_scan();
    DEVICE_ADC_init();
    DEVICE_ADC_read_registers();
    //DEVICE_RTC_set_rtctime();
    DEVICE_ADC_print_voltages(&adc_voltages);
    DEVICE_RTC_print_datetime(&rtc_date_time);
    sprintf(timestamp,"20%02d-%02d-%02dT%02d:%02d:%02d",
                                                                    rtc_date_time.year,
                                                                    rtc_date_time.month,
                                                                    rtc_date_time.day,
                                                                    rtc_date_time.hour,
                                                                    rtc_date_time.min,
                                                                    rtc_date_time.sec
    );
    if ( strptime(timestamp, "%Y-%m-%dT%H:%M:%S", &tm) != NULL ){
        epoch = mktime(&tm);
        printf("UNIX EPOCH: %lld",epoch);
        now.tv_sec=epoch;
        now.tv_usec=0;
        settimeofday(&now,NULL);
    } else {
        ESP_LOGE(TAG_I2C,"Error setting time");
    }


    
    ESP_LOGI(TAG_MCPWM,"initializing MCPWM...");
    if (PERIPH_mcpwm_init() != ESP_OK)
    {
        ESP_LOGE(TAG_MCPWM,"mcpwm start failed");
    }

        ESP_LOGI(TAG_SPI,"initializing SPI...");
    if (PERIPH_spi_master_init() != ESP_OK)
    {
        ESP_LOGE(TAG_SPI,"spi bus init failed");
    }
    ESP_LOGI(TAG_SPI,"initializing SPI IMU...");
    if (DEVICE_IMU_spi_init(&imu) != ESP_OK)
    {
        ESP_LOGE(TAG_SPI,"spi imu init failed");
    }
    ESP_LOGI(TAG_SPI,"starting SPI IMU...");
    if (DEVICE_IMU_start(imu) != ESP_OK)
    {
        ESP_LOGE(TAG_SPI,"spi imu start failed");
    }
    DEVICE_IMU_read_registers(imu);

    ESP_LOGI(TAG_SPI,"initializing SDMCC card...");
    if (DEVICE_sdcard_init() != ESP_OK)
    {
        ESP_LOGE(TAG_SPI,"sdcard init failed");
    }

    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(500/portTICK_PERIOD_MS);
    gpio_set_level(BUZZER_PIN, 0);

    interruptQueue=xQueueCreate(10,sizeof(int));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(TRIGGER_PIN, ISR_handleTriggerInterrupt ,(void*)TRIGGER_PIN);

    xTaskCreate(TASK_DataLogger,"TASK_DataLogger",8192,NULL,1,NULL);
    xTaskCreate(TASK_TriggerRead,"TASK_TriggerRead",2048,NULL,1,NULL);

}