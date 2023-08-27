#include "driver/spi_common.h"
#include "driver/spi_master.h"

#include "h160-defines.h"

extern uint8_t * spi_rxbuf;
extern uint8_t * spi_txbuf;

spi_device_handle_t imu;

acc_gyr_t acc_gyr;

esp_err_t DEVICE_IMU_spi_init(spi_device_handle_t *handle)
{
    esp_err_t ret;

    spi_device_interface_config_t conf;
    memset(&conf, 0, sizeof(spi_device_interface_config_t));
    conf.spics_io_num = SPI_CS_IMU_PIN;
    conf.mode = 0;
    conf.queue_size = 8;
    conf.clock_source = SPI_CLK_SRC_DEFAULT;
    conf.clock_speed_hz = SPI_MASTER_FREQ_8M;

    ret=spi_bus_add_device(SPI2_HOST,&conf,handle);
    return ret;

}

esp_err_t DEVICE_IMU_start(spi_device_handle_t handle){

    esp_err_t ret;

    memset(spi_txbuf,0,SPI_BUFFER_LEN*sizeof(uint8_t));
    memset(spi_rxbuf,0,SPI_BUFFER_LEN*sizeof(uint8_t));
    spi_txbuf[0] = 0x10;
    spi_txbuf[1] = 0b01000000; // start gyroscope 104 Hz
    spi_txbuf[2] = 0b01000000; // start accelerometer 104 Hz

    spi_transaction_t trans;
    memset(&trans,0,sizeof(spi_transaction_t));
    trans.tx_buffer=spi_txbuf;
    trans.rx_buffer=spi_rxbuf;
    trans.length=3*8;
    ret=spi_device_polling_transmit(handle,&trans);

    return ret;
}

esp_err_t DEVICE_IMU_read_acc_gyr(spi_device_handle_t handle, acc_gyr_t *acc_gyr){
    esp_err_t ret;

    spi_txbuf[0] = 0x1<<7|0x22; // Read from register 0x22h (3 gyro 3 acc values, int16_t)

    spi_transaction_t trans;
    memset(&trans,0,sizeof(spi_transaction_t));
    trans.tx_buffer=spi_txbuf;
    trans.rx_buffer=spi_rxbuf;
    trans.length=14*8;
    trans.rxlength=13*8;
    ret=spi_device_transmit(handle,&trans);
    
    memcpy(acc_gyr,spi_rxbuf+0x1,12);

    return ret; 
    
}


esp_err_t DEVICE_IMU_read_registers(spi_device_handle_t handle){
    printf("IMU REGISTER CONTENTS:\n");
    int i;
    esp_err_t ret;

    memset(spi_txbuf,0,SPI_BUFFER_LEN*sizeof(uint8_t));
    memset(spi_rxbuf,0xff,SPI_BUFFER_LEN*sizeof(uint8_t));
    spi_txbuf[0] = 0x1<<7;

    spi_transaction_t trans;
    memset(&trans,0,sizeof(spi_transaction_t));
    trans.tx_buffer=spi_txbuf;
    trans.rx_buffer=spi_rxbuf;
    trans.length=128*8;
    trans.rxlength=127*8;
    ret=spi_device_polling_transmit(handle,&trans);
    
    for(i=0;i<128;i++){
       if(i%16 == 0){
            printf("0x%02x",i);
        }
        if(i%2==0){
            printf(" ");
        }
        printf("%02x",spi_rxbuf[i+1]);
        if(i%16 == 15){
            printf("\n");
        }
    }
    printf("\n");

    int16_t values[7];
    memcpy(values,spi_rxbuf+0x21,14);
    printf("Temp:   %d\n",values[0]);
    printf("GyroX:  %d\n",values[1]);
    printf("GyroY:  %d\n",values[2]);
    printf("GyroZ:  %d\n",values[3]);
    printf("AccelX: %d\n",values[4]);
    printf("AccelY: %d\n",values[5]);
    printf("AccelZ: %d\n",values[6]);

    return ret; 
    
}
