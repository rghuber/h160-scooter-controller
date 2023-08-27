#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "h160-defines.h"
#include "driver/gpio.h"

QueueHandle_t interruptQueue;
volatile int numberOfButtonInterrupts = 0;
volatile bool lastState;
volatile uint32_t debounceTimeout = 0;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ISRs
void IRAM_ATTR ISR_handleTriggerInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  numberOfButtonInterrupts++;
  lastState = gpio_get_level(TRIGGER_PIN);
  debounceTimeout = xTaskGetTickCount(); //version of millis() that works from interrupt
  portEXIT_CRITICAL_ISR(&mux);
}