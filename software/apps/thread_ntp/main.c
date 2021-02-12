// Thread NTP example
//
// Creates a Thread end device which connects to a Thread network. It then uses
// DNS to find the IPv6 address for an NTP server and periodically requests
// time updates from that server.

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
#include "thread_dns.h"
#include "thread_ntp.h"

// Timer data
#define SCHED_QUEUE_SIZE 32
#define SCHED_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE
APP_TIMER_DEF(ntp_update_timer);

// IPv6 data
#define NTP_SERVER_HOSTNAME "us.pool.ntp.org"
#define DNS_SERVER_ADDR "fdaa:bb:1::2"
static otIp6Address ntp_ipv6_address = {0};
static otIp6Address unspecified_ipv6_address = {0};

// Callback upon NTP response
void ntp_response_handler(void* context, uint64_t time, otError error) {
  if (error == OT_ERROR_NONE) {
    printf("Got NTP update: %lu\n", (uint32_t)time);
  } else {
    printf("Got NTP error: %d\n", error);
  }
}

// Callback for timer
void ntp_timer_callback(void* context) {
  otInstance* thread_instance = thread_get_instance();

  if (otIp6IsAddressEqual(&ntp_ipv6_address, &unspecified_ipv6_address)) {
    // If IPv6 address of the ntp server is unspecified try to resolve hostname.
    printf("Resolving IP address with DNS server\n");
    otError error = thread_dns_hostname_resolve(thread_instance,
        DNS_SERVER_ADDR, NTP_SERVER_HOSTNAME,
        dns_response_handler, (void*)&ntp_ipv6_address);
    if (error != OT_ERROR_NONE) {
      printf("DNS ERROR: %d\n", error);
      return;
    }

  } else {
    // Print IPv6 address of NTP server
    printf("Sending NTP poll to: ");
    for (size_t i=0; i<14; i+=2) {
      printf("%02X%02X:", ntp_ipv6_address.mFields.m8[i],
          ntp_ipv6_address.mFields.m8[i+1]);
    }
    printf("%02X%02X\n", ntp_ipv6_address.mFields.m8[14],
        ntp_ipv6_address.mFields.m8[15]);

    // Send NTP request
    otError error = thread_ntp_request(thread_instance, &ntp_ipv6_address,
        NULL, ntp_response_handler);
    if (error != OT_ERROR_NONE) {
      printf("NTP ERROR: %d\n", error);
      return;
    }
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

  // Start a timer to request NTP update every 10 seconds
  app_timer_create(&ntp_update_timer, APP_TIMER_MODE_REPEATED, ntp_timer_callback);
  app_timer_start(ntp_update_timer, APP_TIMER_TICKS(10000), NULL);

  while (true) {
    thread_process();
    app_sched_execute();
    thread_sleep();
  }
}

