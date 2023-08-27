#include "h160-defines.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"

extern spi_device_handle_t imu;
extern acc_gyr_t acc_gyr;

esp_err_t DEVICE_IMU_spi_init(spi_device_handle_t *handle);
esp_err_t DEVICE_IMU_start(spi_device_handle_t handle);
esp_err_t DEVICE_IMU_read_acc_gyr(spi_device_handle_t handle, acc_gyr_t *acc_gyr);
esp_err_t DEVICE_IMU_read_registers(spi_device_handle_t handle);