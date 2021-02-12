OpenThread Border Router
========================

This border router has two parts:

 1. OpenThread Radio Co-Processor (RCP). This runs on an nRF52840DK and
    provides 802.15.4 communication.

 2. Border Router Docker image. This runs on a host computer and provides
    Thread border router functionality.

## nRF52840DK Setup

## Upload code

Flash the border router image to an nRF52840DK.

```shell
$ JLinkExe -device nrf52840_xxaa -if swd -speed 4000 flash.jlink
```

## Attaching the RCP

After flashing, the RCP communicates over the "nRF USB" port on the left side
of the dev kit (**not** the normal USB port which connects to JTAG). You also
have to flip the "nRF Power Switch" in the middle of the board to "USB".

No LEDs will turn on when you've plugged it in, but if everything is correct,
it will appear as a device to attach to your VM. The name of the device will
no longer be "Segger", but will now be "OpenThread Device" (or something like
that).

## Unattaching the RCP

**Remember to flip the middle "nRF Power Switch" back to "VDD".** Otherwise the
board won't work when you plug into its normal JTAG USB interface.


## Docker Setup

### Dependencies

To use this you need to install docker and docker compose

Ubuntu instructions:

```shell
$ sudo apt install curl libffi-dev libssl-dev python3 python3-pip
$ curl -sSL https://get.docker.com | sh
$ sudo usermod -aG docker <YOUR_USERNAME_HERE>
$ sudo pip3 install docker-compose
```

### Running the border router

First, unplug all boards. Then only attach the flashed nRF52840DK RCP as
described above. The scripts assume the board they are contacting is located at
`/dev/ttyACM0`. After the border router has started, you can attached other
boards as normal.

Within this directory, run:

```shell
$ sudo docker-compose up
```

If you're restarting it, run `docker-compose down` first as explained below.

It will take it most of a minute to get started. If everything works right,
you'll eventually see messages that look something like:

```
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | Done
otbr_1  | leader
otbr_1  | Done
otbr_1  | Feb  5 07:00:07 3d0a4e81eed4 otbr-agent[137]: [INFO]-MLE-----: Send Advertisement (ff02:0:0:0:0:0:0:1)
otbr_1  | Feb  5 07:00:07 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----: Sent IPv6 UDP msg, len:90, chksum:9138, to:0xffff, sec:no, prio:net
otbr_1  | Feb  5 07:00:07 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----:     src:[fe80:0:0:0:24a0:6b2a:c9d1:b688]:19788
otbr_1  | Feb  5 07:00:07 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----:     dst:[ff02:0:0:0:0:0:0:1]:19788
otbr_1  | Feb  5 07:00:12 3d0a4e81eed4 otbr-agent[137]: [INFO]-MLE-----: Send Advertisement (ff02:0:0:0:0:0:0:1)
otbr_1  | Feb  5 07:00:12 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----: Sent IPv6 UDP msg, len:90, chksum:64c3, to:0xffff, sec:no, prio:net
otbr_1  | Feb  5 07:00:12 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----:     src:[fe80:0:0:0:24a0:6b2a:c9d1:b688]:19788
otbr_1  | Feb  5 07:00:12 3d0a4e81eed4 otbr-agent[137]: [INFO]-MAC-----:     dst:[ff02:0:0:0:0:0:0:1]:19788
otbr_1  | Feb  5 07:00:15 3d0a4e81eed4 otbr-agent[137]: [INFO]-PLAT----: Session socket is ready
otbr_1  | Feb  5 07:00:15 3d0a4e81eed4 otbr-agent[137]: [INFO]-CLI-----: execute command: state 
```


## Stopping the border router

Be gentle. `Ctrl-C` once to stop it, and then wait for it to clean itself up.
Once it is finished run:

```shell
$ sudo docker-compose down
```

