// setup i2c master
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

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

#include "h160-ISR.h"


void TASK_DataLogger(){

    FILE * acc_gyr_f;
    FILE * bat_curr_f;
    //FILE * rtc_date_time_f;

    acc_gyr_f = fopen("/sdcard/acc_gyr.dat", "wb");
    fclose(acc_gyr_f);

    bat_curr_f = fopen("/sdcard/bat_curr.dat", "wb");
    fclose(bat_curr_f);

    while(1){
        
        DEVICE_IMU_read_acc_gyr(imu,&acc_gyr);
        DEVICE_ADC_read_voltages(&adc_voltages);
        DEVICE_RTC_read_rtctime(&rtc_date_time);

        acc_gyr_f = fopen("/sdcard/acc_gyr.dat", "ab");
        fwrite(&rtc_date_time,sizeof(rtc_time_t),1,acc_gyr_f);
        fwrite(&acc_gyr,sizeof(acc_gyr_t),1,acc_gyr_f);
        fclose(acc_gyr_f);
        
        bat_curr_f = fopen("/sdcard/bat_curr.dat", "ab");
        fwrite(&rtc_date_time,sizeof(rtc_time_t),1,bat_curr_f);
        fwrite(&adc_voltages,sizeof(adc_voltage_t),1,bat_curr_f);
        fclose(bat_curr_f);

        vTaskDelay(50/portTICK_PERIOD_MS);
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
    ESP_LOGI(TAG_NVS,"initializing nonvolatile storage...");
    if (nvs_flash_init() != ESP_OK)
    {
        ESP_LOGE(TAG_NVS,"NVS init failed");
    }

    ESP_LOGI(TAG_GPIO,"initializing GPIO pins...");
    if (PERIPH_gpio_init() != ESP_OK)
    {
        ESP_LOGE(TAG_GPIO,"spi bus init failed");
    }
    
    ESP_LOGI(TAG_I2C,"initializing i2c");
    if (PERIPH_i2c_master_init() != ESP_OK){
        ESP_LOGE(TAG_I2C, "i2c init failed");
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