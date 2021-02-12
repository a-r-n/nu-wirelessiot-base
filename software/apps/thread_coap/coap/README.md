CoAP Server Example
===================

Runs a CoAP server using Node.js

## Installation

Install Node.js and Node Package Manager (npm):

```shell
$ sudo apt install nodejs npm
```

Install node libraries:

```shell
$ npm install
```


## Usage

`coap-server.js` is an IPv4 CoAP server. Run it in a window and leave it
running. It prints whenever it receives a message.

```shell
$ ./coap-server.js
```

`coap-client.js` is an IPv4 CoAP client that can test it. First edit the script
to include your own local IP address. Then run it to send a single message to
the server.

```shell
$ ./coap-client.js
```

