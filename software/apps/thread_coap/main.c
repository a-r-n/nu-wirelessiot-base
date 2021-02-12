// Thread CoAP example
//
// Creates a Thread end device which connects to a Thread network. It then
// periodically sends messages to a CoAP server.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_power.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include <openthread/thread.h>
#include "simple_thread.h"
#include "thread_coap.h"

// Timer data
#define SCHED_QUEUE_SIZE 32
#define SCHED_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE
APP_TIMER_DEF(coap_send_timer);

// IPv6 data
// Note: this is a NAT64 translated address. The bottom part is an IPv4 address
// and should be your local host IPv4 address for the computer running the
// border router and CoAP server.
#define COAP_SERVER_ADDR "2001:db8:1:ffff::192.168.158.128"
static otIp6Address coap_ipv6_address = {0};

// Callback for CoAP responses
void coap_response_handler(void* p_context, otMessage* p_message,
    const otMessageInfo* p_message_info, otError error) {

  if (error != OT_ERROR_NONE) {
    printf("CoAP response error: %d\n", error);
  } else {
    printf("Got response: code %s\n", otCoapMessageCodeToString(p_message));
  }
}

// Callback for timer
void coap_timer_callback(void* context) {
  const uint8_t data[] = "hello";
  const char* path = "test";
  otInstance* thread_instance = thread_get_instance();

  otError error = thread_coap_send(thread_instance,
      OT_COAP_CODE_PUT, OT_COAP_TYPE_NON_CONFIRMABLE,
      &coap_ipv6_address, path, data, strlen((char*)data), coap_response_handler);
  if (error != OT_ERROR_NONE) {
    printf("CoAP send error: %d\n", error);
  } else {
    printf("Sent CoAP message: PUT /%s [%s]\n", path, data);
  }
}

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

  // Initialize application timers
  APP_SCHED_INIT(SCHED_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
  app_timer_init();
  
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

  // Start CoAP
  otInstance* thread_instance = thread_get_instance();
  thread_coap_client_init(thread_instance);

  // Initialize CoAP address
  otError error = otIp6AddressFromString(COAP_SERVER_ADDR, &coap_ipv6_address);
  if (error != OT_ERROR_NONE) {
    printf("IPv6 address error: %d\n", error);
  } else {
    printf("CoAP server address: ");
    for (size_t i=0; i<14; i+=2) {
      printf("%02X%02X:", coap_ipv6_address.mFields.m8[i],
          coap_ipv6_address.mFields.m8[i+1]);
    }
    printf("%02X%02X\n", coap_ipv6_address.mFields.m8[14],
        coap_ipv6_address.mFields.m8[15]);
  }

  // Start a timer to send CoAP messages every 5 seconds
  app_timer_create(&coap_send_timer, APP_TIMER_MODE_REPEATED, coap_timer_callback);
  app_timer_start(coap_send_timer, APP_TIMER_TICKS(5000), NULL);

  while (true) {
    thread_process();
    app_sched_execute();
    thread_sleep();
  }
}

