#! /usr/bin/env node

var coap  = require('coap')

// put your own local IP address here to test the CoAP server
var request = coap.request({'host': '192.168.158.128', 'port': 5683, 'method': 'GET'});

request.on('response', function(response) {
  console.log("Response received!");
  console.log("\tAddress:\t" + response.rsinfo.address.toString() + ":" + response.rsinfo.port.toString());
  console.log("\tURL:\t\t" + response.url.toString());
  console.log("\tMessage:\t" + response.payload.toString());
  console.log("\n");
});

request.end("yo");
