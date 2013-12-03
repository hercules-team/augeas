
module Test_OpenVPN =

let server_conf = "
daemon
local 10.0.5.20
port 1194
# TCP or UDP server?
proto udp
;dev tap
dev tun

dev-node MyTap
ca ca.crt
cert server.crt
key server.key  # This file should be kept secret

# Diffie hellman parameters.
dh dh1024.pem

server 10.8.0.0 255.255.255.0
ifconfig-pool-persist ipp.txt

client-config-dir /etc/openvpn/ccd
server-bridge 10.8.0.4 255.255.255.0 10.8.0.50 10.8.0.100
route 10.9.0.0 255.255.255.0
push \"route 192.168.10.0 255.255.255.0\"
learn-address ./script
push \"redirect-gateway\"
push \"dhcp-option DNS 10.8.0.1\"
push \"dhcp-option WINS 10.8.0.1\"
client-to-client
duplicate-cn
keepalive 10 120
tls-auth ta.key 0 # This file is secret
cipher BF-CBC        # Blowfish (default)
;cipher AES-128-CBC   # AES
;cipher DES-EDE3-CBC  # Triple-DES
comp-lzo
max-clients 100
user nobody
group nobody
persist-key
persist-tun
status openvpn-status.log
log         openvpn.log
log-append  openvpn.log
verb 3
mute 20
management 10.0.5.20 1193 /etc/openvpn/mpass
"

test OpenVPN.lns get server_conf =
  {}
  { "daemon" }
  { "local"    = "10.0.5.20" }
  { "port"     = "1194" }
  { "#comment" = "TCP or UDP server?" }
  { "proto"    = "udp" }
  { "#comment" = "dev tap" }
  { "dev"      = "tun" }
  {}
  { "dev-node" = "MyTap" }
  { "ca"       = "ca.crt" }
  { "cert"     = "server.crt" }
  { "key"      = "server.key"
      { "#comment" = "This file should be kept secret" } }
  {}
  { "#comment" = "Diffie hellman parameters." }
  { "dh"       = "dh1024.pem" }
  {}
  { "server"
      { "address" = "10.8.0.0" }
      { "netmask" = "255.255.255.0" } }
  { "ifconfig-pool-persist" = "ipp.txt" }
  {}
  { "client-config-dir" = "/etc/openvpn/ccd" }
  { "server-bridge"
      { "address" = "10.8.0.4" }
      { "netmask" = "255.255.255.0" }
      { "start"   = "10.8.0.50" }
      { "end"     = "10.8.0.100" } }
  { "route"
      { "address" = "10.9.0.0" }
      { "netmask" = "255.255.255.0" } }
  { "push" = "route 192.168.10.0 255.255.255.0" }
  { "learn-address" = "./script" }
  { "push" = "redirect-gateway" }
  { "push" = "dhcp-option DNS 10.8.0.1" }
  { "push" = "dhcp-option WINS 10.8.0.1" }
  { "client-to-client" }
  { "duplicate-cn" }
  { "keepalive"
      { "ping" = "10" }
      { "timeout" = "120" } }
  { "tls-auth"
      { "key" = "ta.key" }
      { "is_client" = "0" }
      { "#comment" = "This file is secret" } }
  { "cipher" =  "BF-CBC"
      { "#comment" = "Blowfish (default)" } }
  { "#comment" = "cipher AES-128-CBC   # AES" }
  { "#comment" = "cipher DES-EDE3-CBC  # Triple-DES" }
  { "comp-lzo" }
  { "max-clients" = "100" }
  { "user"        = "nobody" }
  { "group"       =  "nobody" }
  { "persist-key" }
  { "persist-tun" }
  { "status"      = "openvpn-status.log" }
  { "log"         = "openvpn.log" }
  { "log-append"  = "openvpn.log" }
  { "verb"        = "3" }
  { "mute"        = "20" }
  { "management"
      { "server"  = "10.0.5.20" }
      { "port"	  = "1193" }
      { "pwfile"  = "/etc/openvpn/mpass" } }



let client_conf = "
client
remote my-server-1 1194
;remote my-server-2 1194
remote-random
resolv-retry infinite
nobind
http-proxy-retry # retry on connection failures
http-proxy mytest 1024
http-proxy mytest2
http-proxy
mute-replay-warnings
ns-cert-type server
"

test OpenVPN.lns get client_conf =
  {}
  { "client" }
  { "remote"
     { "server" = "my-server-1" }
     { "port"   = "1194" } }
  { "#comment"  = "remote my-server-2 1194" }
  { "remote-random" }
  { "resolv-retry" = "infinite" }
  { "nobind" }
  { "http-proxy-retry"
      { "#comment" = "retry on connection failures" } }
  { "http-proxy"
      { "server" = "mytest" }
      { "port"   = "1024" } }
  { "http-proxy"
      { "server" = "mytest2" } }
  { "http-proxy" }
  { "mute-replay-warnings" }
  { "ns-cert-type" = "server" }


