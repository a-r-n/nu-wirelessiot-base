// BLE Scanning app
//
// Receives BLE advertisements

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_timer.h"
#include "ble_db_discovery.h"
#include "ble_lbs_c.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

#include "nrf52840dk.h"

// Scanning/Connecting state
#define APP_BLE_CONN_CFG_TAG  1 // Softdevice BLE configuration
#define APP_BLE_OBSERVER_PRIO 3 // BLE observer priority
NRF_BLE_GATT_DEF(m_gatt); // GATT module instance
NRF_BLE_SCAN_DEF(m_scan); // Scanning module instance
BLE_DB_DISCOVERY_DEF(m_db_disc); // DB discovery module instance
static char const m_target_periph_name[] = "CS397/497"; // Name of the device we try to connect to
BLE_LBS_C_DEF(m_ble_lbs_c); // Structure used by LBS client module

// Function signatures
static void scan_start(void);

/*
// Callback handler for advertisement reception
void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

// extract the fields we care about
ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
uint8_t const* ble_addr = adv_report->peer_addr.addr; // array of 6 bytes of the address
uint8_t* adv_buf = adv_report->data.p_data; // array of up to 31 bytes of advertisement payload data
uint16_t adv_len = adv_report->data.len; // length of advertisement payload data

printf("Received an advertisement!\n");
}
*/
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
            err_code = p_scan_evt->params.connecting_err.err_code;
            APP_ERROR_CHECK(err_code);
            break;
        default:
          break;
    }
}

// handler for GATT discovery
static void db_disc_handler(ble_db_discovery_evt_t * p_evt) {
  //ble_lbs_on_db_disc_evt(&m_ble_lbs_c, p_evt);
}

static void lbs_c_evt_handler(ble_lbs_c_t * p_lbs_c, ble_lbs_c_evt_t * p_lbs_c_evt)
{
    switch (p_lbs_c_evt->evt_type)
    {
        case BLE_LBS_C_EVT_DISCOVERY_COMPLETE:
        {
            ret_code_t err_code;

            err_code = ble_lbs_c_handles_assign(&m_ble_lbs_c,
                                                p_lbs_c_evt->conn_handle,
                                                &p_lbs_c_evt->params.peer_db);
            printf("LED Button service discovered on conn_handle 0x%x.\n", p_lbs_c_evt->conn_handle);

            //err_code = app_button_enable();
            //APP_ERROR_CHECK(err_code);

            // LED Button service discovered. Enable notification of Button.
            err_code = ble_lbs_c_button_notif_enable(p_lbs_c);
            APP_ERROR_CHECK(err_code);
        } break; // BLE_LBS_C_EVT_DISCOVERY_COMPLETE

        case BLE_LBS_C_EVT_BUTTON_NOTIFICATION:
        {
            printf("Button state changed on peer to 0x%x.\n", p_lbs_c_evt->params.button.button_state);
            if (p_lbs_c_evt->params.button.button_state)
            {
                //bsp_board_led_on(LEDBUTTON_LED);
            }
            else
            {
                //bsp_board_led_off(LEDBUTTON_LED);
            }
        } break; // BLE_LBS_C_EVT_BUTTON_NOTIFICATION

        default:
            // No implementation needed.
            break;
    }
}


// handler for all BLE events
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
  ret_code_t err_code;
  printf("BLE event handler called\n");

  // For readability.
  ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

  switch (p_ble_evt->header.evt_id)
  {
    // Upon connection, check which peripheral has connected (HR or RSC), initiate DB
    // discovery, update LEDs status and resume scanning if necessary. */
    case BLE_GAP_EVT_CONNECTED:
      {
        printf("Connected!\n");

        ble_gap_phys_t const phys =
        {
            .rx_phys = BLE_GAP_PHY_2MBPS,
            .tx_phys = BLE_GAP_PHY_2MBPS,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);

        //err_code = ble_lbs_c_handles_assign(&m_ble_lbs_c, p_gap_evt->conn_handle, NULL);
        //APP_ERROR_CHECK(err_code);

        err_code = ble_db_discovery_start(&m_db_disc, p_gap_evt->conn_handle);
        APP_ERROR_CHECK(err_code);

        // Update LEDs status, and check if we should be looking for more
        // peripherals to connect to.
        //bsp_board_led_on(CENTRAL_CONNECTED_LED);
        //bsp_board_led_off(CENTRAL_SCANNING_LED);
      } break;

      // Upon disconnection, reset the connection handle of the peer which disconnected, update
      // the LEDs status and start scanning again.
    case BLE_GAP_EVT_DISCONNECTED:
      {
        printf("Disconnected!\n");
        scan_start();
      } break;

    case BLE_GAP_EVT_TIMEOUT:
      {
        // We have not specified a timeout for scanning, so only connection attemps can timeout.
        if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
        {
          printf("Connection request timed out.\n");
        }
      } break;

    case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
      {
        // Accept parameters requested by peer.
        err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
            &p_gap_evt->params.conn_param_update_request.conn_params);
        APP_ERROR_CHECK(err_code);
      } break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
      {
        printf("PHY update request.\n");
        ble_gap_phys_t const phys =
        {
          .rx_phys = BLE_GAP_PHY_AUTO,
          .tx_phys = BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
      } break;

    case BLE_GATTC_EVT_TIMEOUT:
      {
        // Disconnect on GATT Client timeout event.
        printf("GATT Client Timeout.\n");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
      } break;

    case BLE_GATTS_EVT_TIMEOUT:
      {
        // Disconnect on GATT Server timeout event.
        printf("GATT Server Timeout.\n");
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
            BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
      } break;

    default:
      // No implementation needed.
      break;
  }
}


// Note: this overrides the "weak" ble_stack_init() defined in simple_ble
void ble_stack_init(void)
{
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

static void scan_init(void)
{
  ret_code_t          err_code;
  nrf_ble_scan_init_t init_scan;

  memset(&init_scan, 0, sizeof(init_scan));

  init_scan.connect_if_match = true;
  init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

  err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
  APP_ERROR_CHECK(err_code);

  // Setting filters for scanning.
  err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_NAME_FILTER, false);
  APP_ERROR_CHECK(err_code);

  err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, m_target_periph_name);
  APP_ERROR_CHECK(err_code);
}

static void db_discovery_init(void)
{
  ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
  APP_ERROR_CHECK(err_code);
}

static void scan_start(void)
{
  ret_code_t err_code;

  err_code = nrf_ble_scan_start(&m_scan);
  APP_ERROR_CHECK(err_code);
}

static void power_manage(void) {
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

int main(void) {

  // Initialize power management
  ret_code_t err_code = NRF_SUCCESS;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);

  // APP_TIMER_INIT must be called before BLE setup that relies on it
  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  // Setup BLE and services
  ble_stack_init();
  printf("ble stack setup\n");
  gatt_init();
  printf("gatt setup\n");

  // Initialize scanning and discovery
  scan_init();
  db_discovery_init();

  // Initialize LED button client
  ble_lbs_c_init_t lbs_c_init_obj;
  lbs_c_init_obj.evt_handler = lbs_c_evt_handler;
  err_code = ble_lbs_c_init(&m_ble_lbs_c, &lbs_c_init_obj);
  APP_ERROR_CHECK(err_code);

  // Start scanning
  printf("Starting scanning\n");
  scan_start();

  // go into low power mode
  while(1) {
    power_manage();
  }
}



