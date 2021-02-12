PROJECT_NAME = $(shell basename "$(realpath ./)")

# Configurations
NRF_IC = nrf52840
SDK_VERSION = 15
SOFTDEVICE_MODEL = s140

# Source and header files
APP_HEADER_PATHS += .
APP_SOURCE_PATHS += .
APP_SOURCES = $(notdir $(wildcard ./*.c))

USE_BLE=1

# Path to base of nRF52x-base repo
NRF_BASE_DIR = ../../nrf52x-base/

CFLAGS += -DNRF_MODULE_ENABLE_ALL=1 -DNRF_BLE_SCAN_BUFFER=31 -DNRF_BLE_SCAN_SCAN_INTERVAL=160 \
          -DNRF_BLE_SCAN_SUPERVISION_TIMEOUT=4000 -DNRF_BLE_SCAN_MIN_CONNECTION_INTERVAL=7.5 \
					-DNRF_BLE_SCAN_SCAN_WINDOW=80 -DNRF_BLE_SCAN_SCAN_DURATION=0 \
					-DNRF_BLE_SCAN_MAX_CONNECTION_INTERVAL=30 -DNRF_BLE_SCAN_SLAVE_LATENCY=0 \
					-DNRF_BLE_SCAN_FILTER_ENABLE=1 -DNRF_BLE_SCAN_ADDRESS_CNT=1

APP_SOURCES += $(NRF_BASE_DIR)components/boards/boards.c \
               $(NRF_BASE_DIR)components/libraries/button/app_button.c \
							 $(NRF_BASE_DIR)components/ble/ble_services/ble_lbs_c/ble_lbs_c.c \
							 $(NRF_BASE_DIR)components/ble/nrf_ble_scan/nrf_ble_scan.c \
							 $(NRF_BASE_DIR)components/ble/ble_db_discovery/ble_db_discovery.c \


# Include board Makefile (if any)
include ../../boards/nrf52840dk-ble/Board.mk

# Include main Makefile
include $(NRF_BASE_DIR)/make/AppMakefile.mk
