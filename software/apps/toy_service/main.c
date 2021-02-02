// BLE Service example app
//
// Creates a BLE service and blinks an LED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "simple_ble.h"

#include "nrf52840dk.h"

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
  // c0:98:e5:4e:xx:xx
  .platform_id       = 0x4E,    // used as 4th octect in device BLE address
  .device_id         = 0xAABB,
  .adv_name          = "CS397/497", // used in advertisements if there is room
  .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
  .min_conn_interval = MSEC_TO_UNITS(20, UNIT_1_25_MS),
  .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS),
};

static simple_ble_service_t led_service = {{
  .uuid128 = {0x5f, 0xd5, 0x1f, 0x36, 0x46, 0xef, 0x40, 0xf5,
              0x96, 0x6e, 0xf1, 0xfa, 0xb4, 0x4b, 0x40, 0xda}
}};

// The actual state of the leds (on/off)
static simple_ble_char_t led_state_char = {.uuid16 = 0x4bb5};
static volatile uint8_t led_state = 0;

// The led mask (enable/disable buttons)
static simple_ble_char_t led_mask_char = {.uuid16 = 0x4bb6};
static volatile uint8_t led_mask = 0;

// The led blinking ability (enable/disable blinking)
static simple_ble_char_t led_blink_char = {.uuid16 = 0x4bb7};
static volatile uint8_t led_blink = 0;

static simple_ble_char_t button_char = {.uuid16 = 0x4bb8};
static volatile uint8_t button_state = 0;

/*******************************************************************************
 *   State for this application
 ******************************************************************************/
// Main application state
simple_ble_app_t* simple_ble_app;

void ble_evt_write(ble_evt_t const* p_ble_evt) {

  // Check LED characteristic
  if (simple_ble_is_char_event(p_ble_evt, &led_mask_char)) {
    printf("Received write to LED enable mask : %02x\n", led_mask);
  } else if (simple_ble_is_char_event(p_ble_evt, &led_blink_char)) {
    printf("Received write to LED blink mask : %02x\n", led_blink);
  }
}

static uint64_t approx_elapsed_ms;
static uint64_t last_blink_time;

void read_buttons(void) {
  static uint8_t last_button_state = 0;
  button_state = 0;
  button_state |= !nrf_gpio_pin_read(BUTTON1);
  button_state |= !nrf_gpio_pin_read(BUTTON2) << 1;
  button_state |= !nrf_gpio_pin_read(BUTTON3) << 2;
  button_state |= !nrf_gpio_pin_read(BUTTON4) << 3;

  if (last_button_state != button_state) {
    simple_ble_notify_char(&button_char);
    last_button_state = button_state;
  }
}


void update_leds(void) {
  static const uint16_t blink_ms = 500;
  static uint8_t blink_state = 0;
  static uint8_t last_led_state = 0;

  if (last_blink_time + blink_ms <= approx_elapsed_ms) {
    blink_state = ~blink_state;
    last_blink_time = approx_elapsed_ms;
  }

  led_state = led_mask & (~led_blink | blink_state);

  if (led_state != last_led_state) {
    nrf_gpio_pin_write(LED1, !(led_state & 0b0001));
    nrf_gpio_pin_write(LED2, !(led_state & 0b0010));
    nrf_gpio_pin_write(LED3, !(led_state & 0b0100));
    nrf_gpio_pin_write(LED4, !(led_state & 0b1000));
    last_led_state = led_state;
    printf("LED state changed to 0x%02x\n", led_state);
    simple_ble_notify_char(&led_state_char);
  }
}

int main(void) {

  printf("Board started. Initializing BLE: \n");

  // Setup LED GPIO
  nrf_gpio_cfg_output(LED1);
  nrf_gpio_cfg_output(LED2);
  nrf_gpio_cfg_output(LED3);
  nrf_gpio_cfg_output(LED4);

  // Setup button GPIO
  nrf_gpio_cfg_input(BUTTON1, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON2, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON3, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON4, NRF_GPIO_PIN_PULLUP);

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&led_service);
  simple_ble_add_characteristic(1, 0, 1, 0,
      sizeof(led_state), (uint8_t*)&led_state,
      &led_service, &led_state_char);
  simple_ble_add_characteristic(1, 1, 0, 0,
      sizeof(led_mask), (uint8_t*)&led_mask,
      &led_service, &led_mask_char);
  simple_ble_add_characteristic(1, 1, 0, 0,
      sizeof(led_blink), (uint8_t*)&led_blink,
      &led_service, &led_blink_char);
  simple_ble_add_characteristic(1, 0, 1, 0,
      sizeof(button_state), (uint8_t*)&button_state,
      &led_service, &button_char);

  // Start Advertising
  simple_ble_adv_only_name();

  approx_elapsed_ms = 0;
  last_blink_time = 0;
  led_state = 0;
  led_mask = 0xF;
  led_blink = 0xF;
  while(1) {
    nrf_delay_us(1000);
    approx_elapsed_ms += 1;
    read_buttons();
    update_leds();
  }
}

