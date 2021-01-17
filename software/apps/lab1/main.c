// simple test app

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_serial.h"
#include "nrfx_gpiote.h"

#include "nrf52840dk.h"

/// Update LEDs to match lower 4 bits of counter
void update_leds(uint8_t value) {
  static uint16_t last_value = ~0;
  if (last_value != value) {
    printf("The current value of counter is %d\n", value);
  }
  nrf_gpio_pin_write(LED4, !(value & 0b0001));
  nrf_gpio_pin_write(LED3, !(value & 0b0010));
  nrf_gpio_pin_write(LED2, !(value & 0b0100));
  nrf_gpio_pin_write(LED1, !(value & 0b1000));
  last_value = value;
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // Initialize LED/button
  nrf_gpio_cfg_output(LED1);
  nrf_gpio_cfg_output(LED2);
  nrf_gpio_cfg_output(LED3);
  nrf_gpio_cfg_output(LED4);
  nrf_gpio_cfg_input(BUTTON2, NRF_GPIO_PIN_PULLUP);
  nrf_gpio_cfg_input(BUTTON4, NRF_GPIO_PIN_PULLUP);

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  uint8_t counter = 0;
  bool db = false;
  while (1) {
    nrf_delay_ms(10);

    // Button debounce and counter modification
    bool add_high = !nrf_gpio_pin_read(BUTTON2);
    bool sub_high = !nrf_gpio_pin_read(BUTTON4);
    if (!db) {
      if (add_high) {
        counter++;
        db = true;
        printf("Detected add\n");
      } else if (sub_high) {
        counter--;
        db = true;
        printf("Detected subtract\n");
      }
    } else if (!(add_high || sub_high)){
      db = false;
    }

    counter &= 0xF;
    update_leds(counter);
  }
}
