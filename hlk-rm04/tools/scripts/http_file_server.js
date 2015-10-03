//
// Copyright 2015 The REST Switch Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its
// Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including,
// without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR
// PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any
// risks associated with Your exercise of permissions under this License.
//
// Author: John Clark (johnc@restswitch.com)
//


var port = 80;
var filepath = '../../openwrt-master/bin/ramips/openwrt-ramips-rt305x-hlk-rm04-squashfs-sysupgrade.bin';


var fs   = require('fs');
var http = require('http');
var os   = require('os');
var path = require('path');


http.createServer()
  .on('connection', function(sock) {
    console.log('client connect: \033[32m%s\033[0m', sock.remoteAddress);
  })
  .on('request', function(req, res) {
    var filename = path.basename(filepath);
    console.log('streaming file: \033[32m%s\033[0m to client: \033[32m%s\033[0m', filename, req.connection.remoteAddress);
    res.writeHead(200, {'Content-Disposition': 'attachment; filename="' + filename + '"',
                        'Content-Length': fs.statSync(filepath).size,
                        'Content-Type': 'application/octet-stream'})
    fs.createReadStream(filepath).pipe(res);
  })
  .on('error', function(err) {
    if(err.errno === 'EACCES') {
      console.log('\ncannot open port, please run as root\n');
    }
    else {
      console.log('\n%s\n', err);
    }
  })
  .listen(port);


var interfaces = os.networkInterfaces();
var addresses = [];
for (var i in interfaces) {
  for (var j in interfaces[i]) {
    var address = interfaces[i][j];
    if (address.family === 'IPv4' && !address.internal) {
      addresses.push(address.address);
    }
  }
}

console.log("\nWaiting for connection...");
console.log("  Address%s: [%s]", addresses.length>1?'es':'', addresses.join(' '));
console.log("  Port: [%s]\n", port);

console.log("To upgrade the a140808 device, run the following command:");
console.log("  echo 3 > /proc/sys/vm/drop_caches && /sbin/sysupgrade -v http://%s%s/sysupgrade.bin\n", addresses[0], port!=80?':'+port:'');

console.log("To factory reset the a140808 device, run the following command:");
console.log("  mtd -r erase rootfs_data\n");

