#include "esp_log.h"

#ifndef H160_MCPWM
#define H160_MCPWM


extern uint8_t MOTOR_speed;

esp_err_t PERIPH_mcpwm_init(void);
void DEVICE_MCPWM_run(void);
void DEVICE_MCPWM_stop(void);
void STATE_change_speed(void);

#endif