
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "h160-sdcard.h"
#include "h160-defines.h"

bool sdcard_ready;

esp_err_t DEVICE_sdcard_init(){

    esp_err_t ret;

    sdmmc_card_t *card;

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk   = SDMMC_CLK_PIN;
    slot_config.cmd = SDMMC_CMD_PIN;
    slot_config.d0 = SDMMC_D0_PIN;
    slot_config.d1 = SDMMC_D1_PIN;
    slot_config.d2 = SDMMC_D2_PIN;
    slot_config.d3 = SDMMC_D3_PIN;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG_SDCARD, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG_SDCARD, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    };
    
    sdcard_ready=true;

    ret = sdmmc_get_status(card);
    if (ret != ESP_OK){
       ESP_LOGE(TAG_SDCARD, "sdmmc status abnormal"); 
       sdcard_ready=false;
       return ret;
    } else {
       ESP_LOGI(TAG_SDCARD, "sdmmc status normal"); 
        sdmmc_card_print_info(stdout, card);
    }

    return ret;
}