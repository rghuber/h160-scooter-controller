#include "driver/mcpwm.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "h160-defines.h"
#include "h160-mcpwm.h"

uint8_t MOTOR_speed = 40;

// MCPWM
esp_err_t PERIPH_mcpwm_init(){
    esp_err_t ret;

    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0A,MCPWM_GPIO_A);
    mcpwm_gpio_init(MCPWM_UNIT_0,MCPWM0B,-1);

    mcpwm_config_t conf;
    conf.frequency = MCPWM_FREQ_HZ;
    conf.cmpr_a = 0;
    conf.cmpr_b = 0;
    conf.counter_mode = MCPWM_UP_COUNTER;
    conf.duty_mode = MCPWM_DUTY_MODE_0;
    ret=mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &conf);
    mcpwm_set_signal_low(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM_OPR_A);

    MOTOR_speed = 40;
    return ret;
}

void DEVICE_MCPWM_run(){
    mcpwm_set_duty(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM_OPR_A,(float)MOTOR_speed);
    mcpwm_set_duty_type(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM_OPR_A,MCPWM_DUTY_MODE_0);
}

void DEVICE_MCPWM_stop(){
    mcpwm_set_signal_low(MCPWM_UNIT_0,MCPWM_TIMER_0,MCPWM_OPR_A);
}

void STATE_change_speed(){
    
    if(MOTOR_speed == 40){
        MOTOR_speed=70;
    } else if (MOTOR_speed == 70){
        MOTOR_speed=100;
    } else if (MOTOR_speed == 100){
        MOTOR_speed=40;
    }
    
    switch(MOTOR_speed){
        case 40:
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(100/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            break;
        case 70:
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            break;
        case 100:
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,1);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level(BUZZER_PIN,0);
            break;
        default:
            break;
    }
}
