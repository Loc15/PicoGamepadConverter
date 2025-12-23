#include "mongoose.h"
#include "net.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/watchdog.h"

#include "utils.h"
#include "pico_flash.h"
#include "convert_data.h"

static struct mg_tcpip_if *s_ifp;

uint8_t tud_network_mac_address[6] = {2, 2, 0x84, 0x6A, 0x96, 0};

static const char *dir_web_files = "/web_root";   //directorio de la data

static void blink_cb(void *arg) {  // Blink periodically
  static bool state;
  state ? led_on() : led_off();
  state = !state;
  (void) arg;
}

bool tud_network_recv_cb(const uint8_t *buf, uint16_t len) {
  mg_tcpip_qwrite((void *) buf, len, s_ifp);
  // MG_INFO(("RECV %hu", len));
  // mg_hexdump(buf, len);
  tud_network_recv_renew();
  return true;
}

void tud_network_init_cb(void) {}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
  // MG_INFO(("SEND %hu", arg));
  memcpy(dst, ref, arg);
  return arg;
}

static size_t usb_tx(const void *buf, size_t len, struct mg_tcpip_if *ifp) {
  if (!tud_ready()) return 0;
  while (!tud_network_can_xmit(len)) tud_task();
  tud_network_xmit((void *) buf, len);
  (void) ifp;
  return len;
}

static bool usb_up(struct mg_tcpip_if *ifp) {
  (void) ifp;
  return tud_inited() && tud_ready() && tud_connected();
}


static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    //SET MODE
    if(mg_http_match_uri(hm, "/set_mode")){
      double host, device, bhid;
      //copy the saved old data
      uint8_t buffer[256];
      for (int i = 0; i < TOTAL_SIZE_IN_FLASH; ++i)
      {
        buffer[i] = read_flash(i);
      }
      if(mg_json_get_num(hm->body, "$.host", &host) && mg_json_get_num(hm->body, "$.device", &device)){
          buffer[HOST_MODE_OFFSET] = (uint8_t)host;
          buffer[DEVICE_MODE_OFFSET] = (uint8_t)device;
          //BLUETOOTH MODE
          if(hm->body.len > 21){
            double mac;
            if(mg_json_get_num(hm->body, "$.mac[0]", &mac)){
              buffer[BLUETOOTH_HOST_ADDR_OFFSET] = (uint8_t)mac;
              uint8_t path[10];
              for(int i = 1; i < BLUETOOTH_HOST_ADDR_SIZE; i++){
                sprintf(path, "$.mac[%d]", i);
                mg_json_get_num(hm->body, path, &mac);
                buffer[BLUETOOTH_HOST_ADDR_OFFSET + i] = mac;
              }
            }
            mg_json_get_num(hm->body, "$.bhid", &bhid);
            buffer[BLUETOOTH_HOST_HID_SET_OFFSET] = (uint8_t)bhid;
          }
          //Write to flash
          write_flash(buffer, 1);
          //Reply
          mg_http_reply(c, 200, 0, 0, 0);
          /*RESET*/
          watchdog_reboot(0, SRAM_END, 2000);    
      }
    }
    //SET FEATURES
    else if(mg_http_match_uri(hm, "/set_features")){
      double features = 0;
      //copy the saved old data
      uint8_t buffer[256];
      for (int i = 0; i < TOTAL_SIZE_IN_FLASH; ++i)
      {
        buffer[i] = read_flash(i);
      }

      mg_json_get_num(hm->body, "$.features[0]", &features);
      if(!(uint8_t)features){
        buffer[SET_FEATURE_OFFSET] = 0;
        //Write to flash
        write_flash(buffer, 1);
        //Reply
        mg_http_reply(c, 200, 0, 0, 0);
        /*RESET*/
        watchdog_reboot(0, SRAM_END, 2000); 
      }
      else{
        uint8_t path[15];
        for(int i = 0; i < (SET_FEATURE_SIZE + SIZE_FEATURES_SIZE + FEATURES_SIZE); i++){
          sprintf(path, "$.features[%d]", i);
          mg_json_get_num(hm->body, path, &features);
          buffer[SET_FEATURE_OFFSET + i] = (uint8_t)features;
        }
        for(int i = 0; i < DATA_FEATURES_SIZE; i++){
          sprintf(path, "$.data[%d]", i);
          mg_json_get_num(hm->body, path, &features);
          buffer[DATA_FEATURES_OFFSET + i] = (uint8_t)features;
        }
        //Read buttons bytes from Fast switch mode
        for(int i = 0; i < SWITCH_MODE_BUTTONS_SIZE; i++){
          sprintf(path, "$.data[%d]", 8 + i);
          mg_json_get_num(hm->body, path, &features);
          buffer[SWITCH_MODE_BUTTONS_OFFSET + i] = (uint8_t)features;
        }
        //Write to flash
        write_flash(buffer, 1);
        //Reply
        mg_http_reply(c, 200, 0, 0, 0);
        /*RESET*/
        watchdog_reboot(0, SRAM_END, 2000); 
      }
    }
    else if(mg_http_match_uri(hm, "/read_data")){
      uint8_t buffer[256];
      for (int i = 0; i < TOTAL_SIZE_IN_FLASH; ++i)
      {
        // Void wrong data
        buffer[i] = (read_flash(i) == 0xff ? 0 : read_flash(i));
      }
      // Array to string
      uint8_t size = TOTAL_SIZE_IN_FLASH;
      uint8_t path[size*3];
      uint8_t offset = 0;
      path[0] = '[';
      for(int i = 0; i < size; i++){
        sprintf(&path[1 + offset],"%d,", buffer[i]);
        // strlen don't work
        for (int j = 0; path[j] != '\0'; j++) {
            offset = j;
        }
      }
      path[offset] = ']';
      mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", path);
    }
    else {                                                // For all other URIs,
      struct mg_http_serve_opts opts = {.root_dir = dir_web_files, .fs = &mg_fs_packed};
      mg_http_serve_dir(c, hm, &opts);                      // From root_dir
    }
  }
}

int rndis(void) {
  
  struct mg_mgr mgr;  // Initialise Mongoose event manager
  mg_mgr_init(&mgr);  // and attach it to the interface
  mg_timer_add(&mgr, 500, MG_TIMER_REPEAT, blink_cb, &mgr);

  struct mg_tcpip_driver driver = {.tx = usb_tx, .up = usb_up};
  struct mg_tcpip_if mif = {.mac = {2, 0, 1, 2, 3, 0x77},
                       .ip = mg_htonl(MG_U32(192, 168, 3, 1)),
                       .mask = mg_htonl(MG_U32(255, 255, 255, 0)),
                       .enable_dhcp_server = true,
                       .driver = &driver,
                       .recv_queue.size = 4096};
  s_ifp = &mif;
  mg_tcpip_init(&mgr, &mif);
  tud_init(0);

  mg_http_listen(&mgr, "http://0.0.0.0", fn, NULL);  // Setup listener

  for (;;) {
    mg_mgr_poll(&mgr, 0);
    tud_task();
  }

  return 0;
}
