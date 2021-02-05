Wireshark Application
=====================

Taken from version 3.1.0 of the nRF Sniffer.

Setup guide:
https://infocenter.nordicsemi.com/pdf/nRF_Sniffer_BLE_UG_v3.2.pdf


To upload the Wireshark application to a device:
```
$ JLinkExe -device nrf52840_xxaa -if swd -speed 4000 flash.jlink
```

