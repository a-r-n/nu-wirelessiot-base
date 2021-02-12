Thread CoAP Example App
=======================

Example of using CoAP to communicate over a Thread network with an End Device.
The message "hello" is `PUT` for URL "/test" to a CoAP server running on the
same machine as the border router.

The IPv4 address of the local machine will have to be updated in the
application. To find it:

```shell
$ ip addr
```

The results will look something like (IPv4 address 192.168.158.128):

```
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 00:0c:29:b1:55:09 brd ff:ff:ff:ff:ff:ff
    altname enp2s1
    inet 192.168.158.128/24 brd 192.168.158.255 scope global dynamic noprefixroute ens33
...
```

To receive messages, start the CoAP server in `coap/`, first following the
instructions in that README. You can test that you have the correct IPv4
address by modifying `coap/coap-client.js` with your address and running it
while the server is running.

