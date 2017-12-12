
let ife = require('./build/Debug/IFEBinding').IFE

let a = new ife();

let success = a.up({ name: 'enp10s0', ip: '192.168.178.39', broadcast: '192.168.178.255', netmask: '255.255.255.0', network: '192.168.178.0' })

