
extern QueueHandle_t interruptQueue;
extern volatile int numberOfButtonInterrupts;
extern volatile bool lastState;
extern volatile uint32_t debounceTimeout;
extern portMUX_TYPE mux;

// ISRs
void IRAM_ATTR ISR_handleTriggerInterrupt();