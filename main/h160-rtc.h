

extern rtc_time_t rtc_date_time;

esp_err_t DEVICE_RTC_read_registers();
esp_err_t DEVICE_RTC_set_rtctime();
esp_err_t DEVICE_RTC_read_rtctime(rtc_time_t *rtc_date_time);
void DEVICE_RTC_print_datetime(rtc_time_t *rtc_date_time);
