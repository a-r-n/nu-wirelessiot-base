// BLE Scanning app
//
// Receives BLE advertisements

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "simple_ble.h"

#include "nrf52840dk.h"

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
        // BLE address is c0:98:e5:4e:00:02
        .platform_id       = 0x4E,    // used as 4th octet in device BLE address
        .device_id         = 0x0002,  // used as the 5th and 6th octet in the device BLE address, you will need to change this for each device you have
        .adv_name          = "CS397/497", // irrelevant in this example
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
        .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
        .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;


// Callback handler for advertisement reception
void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

  // extract the fields we care about
  ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
  uint8_t const* ble_addr = adv_report->peer_addr.addr; // array of 6 bytes of the address
  uint8_t* adv_buf = adv_report->data.p_data; // array of up to 31 bytes of advertisement payload data
  uint16_t adv_len = adv_report->data.len; // length of advertisement payload data

  if (ble_addr[5] == 0xc0 &&
      ble_addr[4] == 0x98 &&
      ble_addr[3] == 0xe5)
  {
    // Print bytes
    printf("Found desired device\nRaw payload:\n");
    for (int i = 0; i < adv_len; i++) {
      printf("%x ", adv_buf[i]);
    }
    printf("\n");

    // Print parsed bytes
    for (size_t offset = 0; offset < adv_len; offset++) {
      const uint8_t* base = adv_buf + offset;
      const uint8_t ad_len = base[0];
      const uint8_t ad_type = base[1];
      const uint8_t* data = base + 2;
      switch (ad_type) {
        case 0x1: {
          printf("Flags: 0x%x\n", *data);
          break;
        }
        case 0x9: {
          printf("Name: %.*s\n", ad_len - 1, data);
          break;
        }
        case 0xff: {
          printf("MSD (as string): %s\n", data);
          printf("MSD (raw) \n");
          goto print_adv_bytes;
        }
        default: {
          printf("Unrecognized AD type (0x%x): ", ad_type);
print_adv_bytes:
          for (size_t i = 0; i < ad_len - 1; i++) {
            printf("%x ", data[i]);
          }
          printf("\n");
        }
      }
      offset += ad_len;
    }
    printf("\n");
  }
}


int main(void) {

  // Setup BLE
  // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
  simple_ble_app = simple_ble_init(&ble_config);
  advertising_stop();

  // Start scanning
  scanning_start();

  // go into low power mode
  while(1) {
    power_manage();
  }
}



