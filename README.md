# IFE #

IFE is (network) interface management for Node.js.  It works on Linux, Illumos, FreeBSD, MacOS X.

    var IFEdriver = require('ife'),
        ife = new IFEdriver();

## API ##

---

#### .up(options) ####

Brings up the specified address on the interface "eth0".

**For IPV4:**

| Parameter   | Property      | Type   | Description                            |
| ----------- | ------------- | ------ | -------------------------------------- |
| **options** | **name**      | string | The name of the interface              |
|             | **ip**        | string | The ip (v4) of the interface           |
|             | **broadcast** | string | The broadcast address of the interface |
|             | **netmask**   | string | The netmask of the interface           |
|             | **network**   | string | The network address of the interface   |

##### Example #####
```js
    let options = {
        name:      "eth0",
        ip:        "10.10.10.11",
        broadcast: "10.10.10.255",
        netmask:   "255.255.255.0",
        network:   "10.10.10.0"
    }

    let success = ife.up(options)
```
**For IPV6:**

| Property      | Type   | Description                  |
| ------------- | ------ | ---------------------------- |
| **name**      | string | The name of the interface    |
| **ip**        | string | The ip (v6) of the interface |
| **prefixlen** | number | The prefix length for the ip |

##### Example #####
```js
    let options = {
        name: "e1000g0",
        ip: "2607:f8b0:4002:c09::64",
        prefixlen: 64
    }

    let success = ife.up(options)
```

**Notice**: IPv4 addresses have a subnet mask but instead of typing something
like `255.255.255.0` we use a prefix length for IPv6.

Here is an example of an IPv6 prefix: `2001:1111:2222:3333::/64`.
This is pretty much the same as using `192.168.1.1 /24`.

The number behind the `/` are the number of bits that we use for the prefix.

In the example above it means that `2001:1111:2222:3333` is the prefix (64 bits)
and everything behind it can be used for hosts.

---

#### .down(ip) ####

Brings down a logical interface with a IP address.
The interface name will be resolved through the ip address

##### Example #####

```js
    // brings down the logical ip 10.10.10.11
    let success = ife.down('10.10.10.11')
```

---

#### .list() ####

Lists all the broadcast-capable interfaces on the server.

##### Example #####

```js
    let ifaces = ife.list()

    console.log(ifaces)
    // [
    //   {
    //      name: 'enp10s0',
    //      ip: '192.168.178.39',
    //      broadcast: '192.168.178.255',
    //      netmask: '255.255.255.0',
    //      mac: '00:00:00:00:00:00'
    //   },
    //   {
    //      name: 'docker0',
    //      ip: '172.17.0.1',
    //      broadcast: '172.17.255.255',
    //      netmask: '255.255.0.0',
    //      mac: '00:00:00:00:00:00'
    //   },
    //   {
    //      name: 'enp10s0',
    //      ip: 'fe80::7160:dd80:88af:cee5',
    //      prefixlen: 64,
    //      mac: '00:00:00:00:00:00'
    //   }
    // ]

```

---

#### .arpcache() ####

Returns an hash of IPs and their corresponding MAC addresses in the local server's ARP table.

##### Example #####

```js
    let arpcache = ife.arpcache();

    console.log(arpcache)
    // { '192.168.178.22': '00:00:00' }
```

---

#### .gratarp(options, count, ping) ####

| Parameter   | Property       | Type    | Description                                | Optional |
| ----------- | -------------- | ------- | ------------------------------------------ | -------- |
| **options** | **name**       | string  | The interface name                         | **No**   |
|             | **local_ip**   | string  | Your ip address                            | **No**   |
|             | **remote_ip**  | string  | The remote ip address                      | **No**   |
|             | **remote_mac** | string  | The remote mac address                     | **Yes**  |
| count       |                | number  | How many gratitoucios ARP Requests to send | **No**   |
| ping        |                | boolean | Ping the destination address               | **Yes**  |

##### Example #####

Send 2 (two) gratuitous **ARP** responses to `10.10.10.1` advertising our `10.10.10.11`.
Second, send the same, but explicitly to the target MAC address.
By specifying a MAC address, we may also ping, which we elect to do.

```js
    let count = 2
    let ping = true

    let sent = ife.gratarp({
        name: "eth0",
        local_ip: "10.10.10.11",
        remote_ip: "10.10.10.1"
    }, count)

    // Send the same gratitoucios ARP Request to a specific mac address
    // and ping the address
    let sent = ife.gratarp({
        name: "eth0",
        local_ip: "10.10.10.11",
        remote_ip: "10.10.10.1",
        remote_mac: "7c:d1:c3:dc:dd:f7"
        }, count, ping)
```

