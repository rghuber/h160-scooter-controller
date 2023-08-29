#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_uuid.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "h160-sdcard.h"
#include "h160-bluetooth.h"
#include "h160-defines.h"

esp_err_t ret;
FILE * fp;
static const char *tag = "NimBLE";
static uint8_t own_addr_type;
uint16_t notification_handle;
uint16_t conn_handle;

//!! 5853a5f5-730a-40b2-9b3c-061ff33106f2 data transfer service
static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0xf2, 0x06, 0x31, 0xf3, 0x1f, 0x06, 0x3c, 0x9b, 0xb2, 0x40, 0x0a, 0x73, 0xf5, 0xa5, 0x53, 0x58);

//!! 33b865a1-b9ac-4ff7-8f50-f47fb96591ec command write
static const ble_uuid128_t gatt_svr_chr_cmd_uuid =
    BLE_UUID128_INIT(0xec, 0x91, 0x65, 0xb9, 0x7f, 0xf4, 0x50, 0x8f, 0xf7, 0x4f, 0xac, 0xb9, 0xa1, 0x65, 0xb8, 0x33);

//4d82aa67-bcae-4e7a-bb0e-c5533d8a8488 data read
static const ble_uuid128_t gatt_svr_chr_dat_uuid =
    BLE_UUID128_INIT(0x88, 0x84, 0x8a, 0x3d, 0x53, 0xc5, 0x0e, 0xbb, 0x7a, 0x4e, 0xae, 0xbc, 0x67, 0xaa, 0x82, 0x4d);


char characteristic_value[256];
char characteristic_received_value[256];
char fname[256];
char ** flist;
long int flen;
long int readpos;
uint8_t dl_mode;

uint16_t min_length = 1;   
uint16_t max_length = 255;



//@_____________Forward declaration of some functions ___________
void ble_store_config_init(void);
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);

static int gatt_svr_chr_cmd_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt,
                               void *arg); // command callback function

static int gatt_svr_chr_dat_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt,
                               void *arg); // data callback function

static int gatt_svr_chr_cmd_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len); // command write callback function

static void bleprph_on_reset(int reason);
void bleprph_host_task(void *param);
static void bleprph_on_sync(void);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {

        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){{
                                                           .uuid = &gatt_svr_chr_cmd_uuid.u,     // command characteristic UUID
                                                           .access_cb = gatt_svr_chr_cmd_access, // command callback
                                                           .val_handle = &notification_handle,
                                                           .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE, // permissions
                                                       },
                                                       {
                                                           .uuid = &gatt_svr_chr_dat_uuid.u,     // data characterisitc UUID
                                                           .access_cb = gatt_svr_chr_dat_access, // data callback
                                                           .val_handle = &notification_handle,
                                                           .flags = BLE_GATT_CHR_F_READ, // permissions
                                                       },
                                                       {
                                                           0, /* No more characteristics in this service. This is necessary */
                                                       }},
    },

    {
        0, /* No more services. This is necessary */
    },
};

static int gatt_svr_chr_cmd_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt,
                               void *arg)  // command callback
{

  int rc;

  switch (ctxt->op)
  {
  case BLE_GATT_ACCESS_OP_READ_CHR: //!! In case user accessed this characterstic to read its value, bellow lines will execute
    rc = os_mbuf_append(ctxt->om, &characteristic_value,
                        sizeof characteristic_value);
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

  case BLE_GATT_ACCESS_OP_WRITE_CHR: //!! In case user accessed this characterstic to write, bellow lines will executed.
    rc = gatt_svr_chr_cmd_write(ctxt->om, min_length, max_length, &characteristic_received_value, NULL); //!! Function "gatt_svr_chr_cmd_write" will fire.
    //printf("Received=%s\n", characteristic_received_value);  // Print the received value
    //! Use received value in you code. For example
    if(characteristic_received_value[0] == 'l'){
        printf("received l, listing directory\n");
        if(sdcard_ready){
          DIR* dir = opendir("/sdcard");
          flist=(char**)malloc(256*sizeof(char*));
          for(int i = 0;i<256;i++){
              flist[i]=(char*)malloc(256*sizeof(char));
          }
          int i=0;
          while (true) {
              struct dirent* de = readdir(dir);
              if (!de) {
                break;
              }
              if(de->d_name[0] != '.' && i < 256){
                strcpy(flist[i],de->d_name);
                i++;
              }
          }
          flen=i;
          readpos=0;
          for(i=0;i<flen;i++){
            //printf("File %03d: %s\n",i,flist[i]);
            sprintf(characteristic_value,"%ld files",flen);
          }
          dl_mode = DL_MODE_DIRLIST;
        } else {
          sprintf(characteristic_value,"0 files"); 
        }

    } else if (characteristic_received_value[0] == 'd') {
      if(sdcard_ready){
        printf("received d, providing file %s\n",characteristic_received_value+2);
        char prefix[]="/sdcard/";
        memcpy(fname,prefix,8);
        memcpy(fname+8,characteristic_received_value+2,248);
        struct stat st;
        stat(fname, &st);
        long int size = st.st_size;
        sprintf(characteristic_value,"%ld bytes",size);
        flen=(size>>8)+1;
        readpos=0;
        fp=fopen(fname,"rb");
        dl_mode = DL_MODE_FILE;
      } else {
          sprintf(characteristic_value,"0 blocks"); 
      }


    } else if (characteristic_received_value[0] == 'r') {
      if(sdcard_ready){
        char prefix[]="/sdcard/";
        memcpy(fname,prefix,8);
        memcpy(fname+8,characteristic_received_value+2,248);
        printf("received r, removing file %s\n",fname);
        if(remove(fname)==0){
            printf("deleted file %s\n",fname);
            sprintf(characteristic_value,"deleted 1 file"); 
        } else {
            printf("failed to delete file %s\n",fname);
        }
      } else {
            sprintf(characteristic_value,"deleted 0 files"); 
      }
    }

    return rc;
  default:
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
  }
}

static int gatt_svr_chr_dat_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt,
                               void *arg)  //!! Callback function. When ever characrstic will be accessed by user, this function will execute
{

  int rc;

  switch (ctxt->op)
  {
  case BLE_GATT_ACCESS_OP_READ_CHR: //!! In case user accessed this characterstic to read its value, bellow lines will execute
    
    switch(dl_mode){
        case DL_MODE_DIRLIST:
            //printf("reading position %ld, file %s\n",readpos,flist[readpos]);
            memset(characteristic_value,0,256);
            strcpy(characteristic_value,flist[readpos]);
            readpos++;
            if(readpos == flen){
                dl_mode = DL_MODE_VOID;
            }
            rc = os_mbuf_append(ctxt->om, &characteristic_value,
                        sizeof characteristic_value);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        case DL_MODE_FILE:
            //printf("reading block %ld, file %s\n",readpos,fname);
            memset(characteristic_value,0,256);
            fread(characteristic_value,256,1,fp);
            readpos++;
            if(readpos == flen){
                dl_mode = DL_MODE_VOID;
                fclose(fp);
            }
            rc = os_mbuf_append(ctxt->om, &characteristic_value,
                        sizeof characteristic_value);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            rc = os_mbuf_append(ctxt->om, &characteristic_value,
                        sizeof characteristic_value);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        default:
            memset(characteristic_value,0,256); 
            rc = os_mbuf_append(ctxt->om, &characteristic_value,
                        sizeof characteristic_value);
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }

  default:
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
  }
}


void startBLE() //! Call this function to start BLE
{

  //! Below is the sequence of APIs to be called to init/enable NimBLE host and ESP controller:
  printf("\n Staring BLE \n");
  int rc;


  ret=nimble_port_init();
  ESP_ERROR_CHECK(ret);
  /* Initialize the NimBLE host configuration. */
  ble_hs_cfg.reset_cb = bleprph_on_reset;
  ble_hs_cfg.sync_cb = bleprph_on_sync;
  ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
  ble_hs_cfg.sm_io_cap = 3;
  ble_hs_cfg.sm_sc = 0;

  rc = gatt_svr_init();
  assert(rc == 0);

  rc = ble_svc_gap_device_name_set(SCOOTER_NAME); //!! Set the name of this device
  assert(rc == 0);

  nimble_port_freertos_init(bleprph_host_task);
}

static int gatt_svr_chr_cmd_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len, void *dst, uint16_t *len)
{
  uint16_t om_len;
  int rc;

  om_len = OS_MBUF_PKTLEN(om);
  if (om_len < min_len || om_len > max_len)
  {
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
  }

  rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
  if (rc != 0)
  {
    return BLE_ATT_ERR_UNLIKELY;
  }

  return 0;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
  char buf[BLE_UUID_STR_LEN];

  switch (ctxt->op)
  {
  case BLE_GATT_REGISTER_OP_SVC:
    MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                ctxt->svc.handle);
    break;

  case BLE_GATT_REGISTER_OP_CHR:
    MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                       "def_handle=%d val_handle=%d\n",
                ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                ctxt->chr.def_handle,
                ctxt->chr.val_handle);
    break;

  case BLE_GATT_REGISTER_OP_DSC:
    MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                ctxt->dsc.handle);
    break;

  default:
    assert(0);
    break;
  }
}

int gatt_svr_init(void)
{
  int rc;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(gatt_svr_svcs);
  if (rc != 0)
  {
    return rc;
  }

  rc = ble_gatts_add_svcs(gatt_svr_svcs);
  if (rc != 0)
  {
    return rc;
  }

  return 0;
}

static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
  MODLOG_DFLT(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=",
              desc->conn_handle, desc->our_ota_addr.type);
  print_addr(desc->our_ota_addr.val);
  MODLOG_DFLT(INFO, " our_id_addr_type=%d our_id_addr=",
              desc->our_id_addr.type);
  print_addr(desc->our_id_addr.val);
  MODLOG_DFLT(INFO, " peer_ota_addr_type=%d peer_ota_addr=",
              desc->peer_ota_addr.type);
  print_addr(desc->peer_ota_addr.val);
  MODLOG_DFLT(INFO, " peer_id_addr_type=%d peer_id_addr=",
              desc->peer_id_addr.type);
  print_addr(desc->peer_id_addr.val);
  MODLOG_DFLT(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                    "encrypted=%d authenticated=%d bonded=%d\n",
              desc->conn_itvl, desc->conn_latency,
              desc->supervision_timeout,
              desc->sec_state.encrypted,
              desc->sec_state.authenticated,
              desc->sec_state.bonded);
}

static void
bleprph_advertise(void)
{
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  const char *name;
  int rc;

  /**
   *  Set the advertisement data included in our advertisements:
   *     o Flags (indicates advertisement type and other general info).
   *     o Advertising tx power.
   *     o Device name.
   *     o 16-bit service UUIDs (alert notifications).
   */

  memset(&fields, 0, sizeof fields);

  /* Advertise two flags:
   *     o Discoverability in forthcoming advertisement (general)
   *     o BLE-only (BR/EDR unsupported).
   */
  fields.flags = BLE_HS_ADV_F_DISC_GEN |
                 BLE_HS_ADV_F_BREDR_UNSUP;

  /* Indicate that the TX power level field should be included; have the
   * stack fill this value automatically.  This is done by assigning the
   * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
   */
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  name = ble_svc_gap_device_name();
  fields.name = (uint8_t *)name;
  fields.name_len = strlen(name);
  fields.name_is_complete = 1;

  fields.uuids16 = (ble_uuid16_t[]){
      BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)};
  fields.num_uuids16 = 1;
  fields.uuids16_is_complete = 1;

  rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0)
  {
    MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
    return;
  }

  /* Begin advertising. */
  memset(&adv_params, 0, sizeof adv_params);
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                         &adv_params, bleprph_gap_event, NULL);
  if (rc != 0)
  {
    MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
    return;
  }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
  struct ble_gap_conn_desc desc;
  int rc;

  switch (event->type)
  {
  case BLE_GAP_EVENT_CONNECT:
    /* A new connection was established or a connection attempt failed. */
    MODLOG_DFLT(INFO, "connection %s; status=%d ",
                event->connect.status == 0 ? "established" : "failed",
                event->connect.status);
    if (event->connect.status == 0)
    {
      rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
      assert(rc == 0);
      bleprph_print_conn_desc(&desc);
    }
    MODLOG_DFLT(INFO, "\n");

    if (event->connect.status != 0)
    {
      /* Connection failed; resume advertising. */
      bleprph_advertise();
    }
    conn_handle = event->connect.conn_handle;
    return 0;

  case BLE_GAP_EVENT_DISCONNECT:
    MODLOG_DFLT(INFO, "disconnect; reason=%d ", event->disconnect.reason);
    bleprph_print_conn_desc(&event->disconnect.conn);
    MODLOG_DFLT(INFO, "\n");

    /* Connection terminated; resume advertising. */
    bleprph_advertise();
    return 0;

  case BLE_GAP_EVENT_CONN_UPDATE:
    /* The central has updated the connection parameters. */
    MODLOG_DFLT(INFO, "connection updated; status=%d ",
                event->conn_update.status);
    rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
    assert(rc == 0);
    bleprph_print_conn_desc(&desc);
    MODLOG_DFLT(INFO, "\n");
    return 0;

  case BLE_GAP_EVENT_ADV_COMPLETE:
    MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                event->adv_complete.reason);
    bleprph_advertise();
    return 0;

  case BLE_GAP_EVENT_MTU:
    MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                event->mtu.conn_handle,
                event->mtu.channel_id,
                event->mtu.value);
    return 0;
  }

  return 0;
}

static void
bleprph_on_reset(int reason)
{
  MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void
bleprph_on_sync(void)
{
  int rc;

  rc = ble_hs_util_ensure_addr(0);
  assert(rc == 0);

  /* Figure out address to use while advertising (no privacy for now) */
  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0)
  {
    MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
    return;
  }

  /* Printing ADDR */
  uint8_t addr_val[6] = {0};
  rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

  MODLOG_DFLT(INFO, "Device Address: ");
  print_addr(addr_val);
  MODLOG_DFLT(INFO, "\n");
  /* Begin advertising. */
  bleprph_advertise();
}

void bleprph_host_task(void *param)
{
  ESP_LOGI(tag, "BLE Host Task Started");
  /* This function will return only when nimble_port_stop() is executed */
  nimble_port_run();

  nimble_port_freertos_deinit();
}

void
print_bytes(const uint8_t *bytes, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        MODLOG_DFLT(INFO, "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

void
print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}