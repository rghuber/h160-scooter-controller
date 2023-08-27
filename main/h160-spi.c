#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "h160-defines.h"

uint8_t * spi_rxbuf;
uint8_t * spi_txbuf;

esp_err_t PERIPH_spi_master_init()
{
    spi_bus_config_t conf;
    memset(&conf, 0, sizeof(spi_bus_config_t));
    conf.miso_io_num = SPI_MISO_PIN;
    conf.mosi_io_num = SPI_MOSI_PIN;
    conf.sclk_io_num = SPI_SCLK_PIN;
    conf.quadhd_io_num = -1;
    conf.quadwp_io_num = -1;
    conf.max_transfer_sz = SPI_BUFFER_LEN*8;

    spi_txbuf = heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);
    spi_rxbuf = heap_caps_malloc(SPI_BUFFER_LEN, MALLOC_CAP_DMA);

    return spi_bus_initialize(SPI2_HOST, &conf, SPI_DMA_CH_AUTO);
}
