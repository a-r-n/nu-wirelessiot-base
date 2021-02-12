// Thread example end device
//
// Creates a Thread end device which connects to a Thread network

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_power.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <openthread/thread.h>
#include "simple_thread.h"

// Callback for thread state changing
void thread_state_changed_callback(uint32_t flags, void* p_context) {
  const char* status = "None";
  otDeviceRole role = otThreadGetDeviceRole(p_context);
  if (role == OT_DEVICE_ROLE_DISABLED) {
    status = "Disabled";
  } else if (role == OT_DEVICE_ROLE_DETACHED) {
    status = "Detached";
  } else if (role == OT_DEVICE_ROLE_CHILD) {
    status = "Child";
  } else if (role == OT_DEVICE_ROLE_ROUTER) {
    status = "Router";
  } else if (role == OT_DEVICE_ROLE_LEADER) {
    status = "Leader";
}
  printf("Thread state changed! Status: %s\n", status);
}


int main(void) {

  // Start DC-DC power supply
  nrf_power_dcdcen_set(1);

  // Initialize logs
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Board started. Initializing Thread\n");

  // Start Thread
  thread_config_t thread_config = {
    .channel = 25,
    .panid = 0xFACE,
    .sed = true,
    .poll_period = 1000, // data request period in milliseconds
    .child_period = 40,  // child timeout in seconds
    .autocommission = true,
  };
  thread_init(&thread_config);

  while (true) {
    thread_process();
    thread_sleep();
  }
}

