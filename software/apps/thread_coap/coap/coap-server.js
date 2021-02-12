#!/usr/bin/env node

var coap = require('coap');
var server6 = coap.createServer({ type: 'udp4' });


server6.on('request', function(request, response) {
  console.log("Message received!");
  console.log("\tAddress:\t" + request.rsinfo.address.toString() + ":" + request.rsinfo.port.toString());
  console.log("\tURL:\t\t" + request.url.toString());
  console.log("\t" + request.method + ":\t\t" + request.payload.toString());
  console.log("\n");
  response.end("HELLO!\n");
});

// Note: the default CoAP port is 5683
server6.listen(function() {
  console.log("CoAP Server Listening...\n");
});

