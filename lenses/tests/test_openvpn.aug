
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
  { "ifconfig-pool-persist"
    { "file" = "ipp.txt" }
  }
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
  { "status"
      { "file" = "openvpn-status.log" }
  }
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
  { "mute-replay-warnings" }
  { "ns-cert-type" = "server" }

(* Most (hopefully all) permutations for OpenVPN 2.3
 * NOTE: This completely ignores IPv6 because it's hard to tell which OpenVPN
 * options actually work with IPv6. Thar be dragons.
 *)
let all_permutations_conf = "
config /a/canonical/file
config relative_file
mode p2p
mode server
local 192.168.1.1
local hostname
remote 192.168.1.1 1234
remote hostname 1234
remote hostname
remote 192.168.1.1
remote hostname 1234 tcp
remote 192.168.1.1 1234 tcp
remote hostname 1234 udp
remote-random-hostname
#comment square <connection> blocks should go here
proto-force udp
proto-force tcp
remote-random
proto udp
proto tcp-client
proto tcp-server
connect-retry 5
connect-timeout 10
connect-retry-max 0
show-proxy-settings
http-proxy servername 1234
http-proxy servername 1234 auto
http-proxy servername 1234 auto-nct
http-proxy servername 1234 auto none
http-proxy servername 1234 auto basic
http-proxy servername 1234 auto ntlm
http-proxy servername 1234 relative_filename ntlm
http-proxy servername 1234 /canonical/filename basic
http-proxy-retry
http-proxy-timeout 5
http-proxy-option VERSION 1.0
http-proxy-option AGENT an unquoted string with spaces
http-proxy-option AGENT an_unquoted_string_without_spaces
socks-proxy servername
socks-proxy servername 1234
socks-proxy servername 1234 /canonical/file
socks-proxy servername 1234 relative/file
socks-proxy-retry
resolv-retry 5
float
ipchange my command goes here
port 1234
lport 1234
rport 1234
bind
nobind
dev tun
dev tun0
dev tap
dev tap0
dev null
dev-type tun
dev-type tap
topology net30
topology p2p
topology subnet
tun-ipv6
dev-node /canonical/file
dev-node relative/file
lladdr 1.2.3.4
iproute my command goes here
ifconfig 1.2.3.4 5.6.7.8
ifconfig-noexec
ifconfig-nowarn
route 111.222.123.123
route networkname
route vpn_gateway
route net_gateway
route remote_host
route 111.222.123.123 255.123.255.221
route 111.222.123.123 default
route 111.222.123.123 255.123.255.231 111.222.123.1
route 111.222.123.123 default 111.222.123.1
route 111.222.123.123 255.123.255.231 default
route 111.222.123.123 default default
route 111.222.123.123 255.123.255.231 gatewayname
route 111.222.123.123 255.123.255.231 gatewayname 5
route 111.222.123.123 255.123.255.231 vpn_gateway
route 111.222.123.123 255.123.255.231 net_gateway
route 111.222.123.123 255.123.255.231 remote_host
route 111.222.123.123 255.123.255.231 111.222.123.1
route 111.222.123.123 255.123.255.231 111.222.123.1 5
max-routes 5
route-gateway gateway-name
route-gateway 111.222.123.1
route-gateway dhcp
route-metric 5
route-delay
route-delay 1
route-delay 1 2
route-up my command goes here
route-pre-down my command goes here
route-noexec
route-nopull
allow-pull-fqdn
client-nat snat 1.2.3.4 5.6.7.8 9.8.7.6
client-nat dnat 1.2.3.4 5.6.7.8 9.8.7.6
redirect-gateway local
redirect-gateway local autolocal
redirect-gateway local autolocal def1 bypass-dhcp bypass-dns block-local
link-mtu 5
redirect-private local
redirect-private local autolocal
redirect-private local autolocal def1 bypass-dhcp bypass-dns block-local
tun-mtu 5
tun-mtu-extra 5
mtu-disc no
mtu-disc maybe
mtu-disc yes
mtu-test
fragment 5
mssfix 1600
sndbuf 65536
rcvbuf 65535
mark blahvalue
socket-flags TCP_NODELAY
txqueuelen 5
shaper 50
inactive 5
inactive 5 1024
ping 10
ping-exit 10
ping-restart 10
keepalive 1 2
ping-timer-rem
persist-tun
persist-key
persist-local-ip
persist-remote-ip
mlock
up my command goes here
up-delay
down my command goes here
down-pre
up-restart
setenv myname myvalue
setenv my0-_name my value with spaces
setenv-safe myname myvalue
setenv-safe my-_name my value with spaces
ignore-unknown-option anopt
ignore-unknown-option anopt anotheropt
script-security 3
disable-occ
user username
group groupname
cd /canonical/dir
cd relative/dir/
chroot /canonical/dir
chroot relative/dir/
setcon selinux-context
daemon
daemon mydaemon_name
syslog
syslog my_syslog-name
errors-to-stderr
passtos
inetd
inetd wait
inetd nowait
inetd wait my-program_name
log myfilename
log-append myfilename
suppress-timestamps
writepid myfile
nice 5
fast-io
multihome
echo stuff to echo until end of line
remap-usr1 SIGHUP
remap-usr1 SIGTERM
verb 6
status myfile
status myfile 15
status-version
status-version 3
mute 20
comp-lzo
comp-lzo yes
comp-lzo no
comp-lzo adaptive
management 123.123.123.123 1234
management 123.123.123.123 1234 /canonical/file
management-client
management-query-passwords
management-query-proxy
management-query-remote
management-forget-disconnect
management-hold
management-signal
management-up-down
management-client-auth
management-client-pf
management-log-cache 5
management-client-user myuser
management-client-user mygroup
plugin /canonical/file
plugin relative/file
plugin myfile an init string
server 1.2.3.4 255.255.255.0
server 1.2.3.4 255.255.255.255 nopool
server-bridge 1.2.3.4 1.2.3.5 50.5.5.5 50.5.5.6
server-bridge nogw
push \"my push string\"
push-reset
push-peer-info
disable
ifconfig-pool 1.1.1.1 2.2.2.2
ifconfig-pool 1.1.1.1 2.2.2.2 255.255.255.0
ifconfig-pool-persist myfile
ifconfig-pool-persist myfile 50
ifconfig-pool-linear
ifconfig-push 1.1.1.1 2.2.2.2
ifconfig-push 1.1.1.1 2.2.2.2 alias-name
iroute 1.1.1.1
iroute 1.1.1.1 2.2.2.2
client-to-client
duplicate-cn
client-connect my command goes here
client-disconnect my command goes here
client-config-dir directory
ccd-exclusive
tmp-dir /directory
hash-size 1 2
bcast-buffers 5
tcp-queue-limit 50
tcp-nodelay
max-clients 50
max-routes-per-client 50
stale-routes-check 5
stale-routes-check 5 50
connect-freq 50 100
learn-address my command goes here
auth-user-pass-verify /my/script/with/no/arguments.sh via-env
auth-user-pass-verify \"myscript.sh arg1 arg2\" via-file
opt-verify
auth-user-pass-optional
client-cert-not-required
username-as-common-name
port-share 1.1.1.1 1234
port-share myhostname 1234
port-share myhostname 1234 /canonical/dir
client
pull
auth-user-pass
auth-user-pass /canonical/file
auth-user-pass relative/file
auth-retry none
auth-retry nointeract
auth-retry interact
static-challenge challenge_no_spaces 1
static-challenge \"my quoted challenge string\" 0
server-poll-timeout 50
explicit-exit-notify
explicit-exit-notify 5
secret /canonicalfile
secret relativefile
secret filename 1
secret filename 0
key-direction
auth none
auth sha1
cipher SHA1
cipher sha1
keysize 50
prng SHA1
prng SHA1 500
engine
engine blah
no-replay
replay-window 64
replay-window 64 16
mute-replay-warnings
replay-persist /my/canonical/filename
no-iv
use-prediction-resistance
test-crypto
tls-server
tls-client
ca myfile
capath /mydir/
dh myfile
cert myfile
extra-certs myfile
key myfile
tls-version-min 1.1
tls-version-min 2
tls-version-min 1.1 or-highest
tls-version-max 5.5
pkcs12 myfile
verify-hash AD:B0:95:D8:09:C8:36:45:12:A9:89:C8:90:09:CB:13:72:A6:AD:16
pkcs11-cert-private 0
pkcs11-cert-private 1
pkcs11-id myname
pkcs11-id-management
pkcs11-pin-cache 50
pkcs11-protected-authentication 0
pkcs11-protected-authentication 1
cryptoapicert \"SUBJ:Justin Akers\"
key-method 2
tls-cipher DEFAULT:!EXP:!PSK:!SRP:!kRSA
tls-timeout 50
reneg-bytes 50
reneg-pkts 50
reneg-sec 5
hand-window 123
tran-window 456
single-session
tls-exit
tls-auth filename 1
askpass /canonical/filename
auth-nocache
tls-verify my command goes here
tls-export-cert /a/directory/for/things
x509-username-field emailAddress
x509-username-field ext:subjectAltName
tls-remote myhostname
verify-x509-name hostname name
verify-x509-name hostname name-prefix
verify-x509-name hostname subject
ns-cert-type server
ns-cert-type client
remote-cert-tls server
remote-cert-tls client
remote-cert-ku 01
remote-cert-ku 01 02 fa FF b3
remote-cert-eku 123.3510.350.10
remote-cert-eku \"TLS Web Client Authentication\"
remote-cert-eku serverAuth
crl-verify /a/file/path
crl-verify /a/directory/ dir
show-ciphers
show-digests
show-tls
show-engines
genkey
mktun
rmtun
ifconfig-ipv6 2000:123:456::/64 1234:99:123::124
ifconfig-ipv6-push 2000:123:456::/64 1234:99:123::124
iroute-ipv6 2000:123:456::/64
route-ipv6 2000:123:456::/64
route-ipv6 2000:123:456::/64 1234:99:123::124
route-ipv6 2000:123:456::/64 1234:99:123::124 500
server-ipv6 2000:123:456::/64
ifconfig-ipv6-pool 2000:123:456::/64

"

test OpenVPN.lns get all_permutations_conf =
  {  }
  { "config" = "/a/canonical/file" }
  { "config" = "relative_file" }
  { "mode" = "p2p" }
  { "mode" = "server" }
  { "local" = "192.168.1.1" }
  { "local" = "hostname" }
  { "remote"
    { "server" = "192.168.1.1" }
    { "port" = "1234" }
  }
  { "remote"
    { "server" = "hostname" }
    { "port" = "1234" }
  }
  { "remote"
    { "server" = "hostname" }
  }
  { "remote"
    { "server" = "192.168.1.1" }
  }
  { "remote"
    { "server" = "hostname" }
    { "port" = "1234" }
    { "proto" = "tcp" }
  }
  { "remote"
    { "server" = "192.168.1.1" }
    { "port" = "1234" }
    { "proto" = "tcp" }
  }
  { "remote"
    { "server" = "hostname" }
    { "port" = "1234" }
    { "proto" = "udp" }
  }
  { "remote-random-hostname" }
  { "#comment" = "comment square <connection> blocks should go here" }
  { "proto-force" = "udp" }
  { "proto-force" = "tcp" }
  { "remote-random" }
  { "proto" = "udp" }
  { "proto" = "tcp-client" }
  { "proto" = "tcp-server" }
  { "connect-retry" = "5" }
  { "connect-timeout" = "10" }
  { "connect-retry-max" = "0" }
  { "show-proxy-settings" }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "auto" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "auto-nct" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "auto" }
    { "auth-method" = "none" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "auto" }
    { "auth-method" = "basic" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "auto" }
    { "auth-method" = "ntlm" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "relative_filename" }
    { "auth-method" = "ntlm" }
  }
  { "http-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "/canonical/filename" }
    { "auth-method" = "basic" }
  }
  { "http-proxy-retry" }
  { "http-proxy-timeout" = "5" }
  { "http-proxy-option"
    { "option" = "VERSION" }
    { "value" = "1.0" }
  }
  { "http-proxy-option"
    { "option" = "AGENT" }
    { "value" = "an unquoted string with spaces" }
  }
  { "http-proxy-option"
    { "option" = "AGENT" }
    { "value" = "an_unquoted_string_without_spaces" }
  }
  { "socks-proxy"
    { "server" = "servername" }
  }
  { "socks-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
  }
  { "socks-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "/canonical/file" }
  }
  { "socks-proxy"
    { "server" = "servername" }
    { "port" = "1234" }
    { "auth" = "relative/file" }
  }
  { "socks-proxy-retry" }
  { "resolv-retry" = "5" }
  { "float" }
  { "ipchange" = "my command goes here" }
  { "port" = "1234" }
  { "lport" = "1234" }
  { "rport" = "1234" }
  { "bind" }
  { "nobind" }
  { "dev" = "tun" }
  { "dev" = "tun0" }
  { "dev" = "tap" }
  { "dev" = "tap0" }
  { "dev" = "null" }
  { "dev-type" = "tun" }
  { "dev-type" = "tap" }
  { "topology" = "net30" }
  { "topology" = "p2p" }
  { "topology" = "subnet" }
  { "tun-ipv6" }
  { "dev-node" = "/canonical/file" }
  { "dev-node" = "relative/file" }
  { "lladdr" = "1.2.3.4" }
  { "iproute" = "my command goes here" }
  { "ifconfig"
    { "local" = "1.2.3.4" }
    { "remote" = "5.6.7.8" }
  }
  { "ifconfig-noexec" }
  { "ifconfig-nowarn" }
  { "route"
    { "address" = "111.222.123.123" }
  }
  { "route"
    { "address" = "networkname" }
  }
  { "route"
    { "address" = "vpn_gateway" }
  }
  { "route"
    { "address" = "net_gateway" }
  }
  { "route"
    { "address" = "remote_host" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.221" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "default" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "111.222.123.1" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "default" }
    { "gateway" = "111.222.123.1" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "default" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "default" }
    { "gateway" = "default" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "gatewayname" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "gatewayname" }
    { "metric" = "5" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "vpn_gateway" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "net_gateway" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "remote_host" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "111.222.123.1" }
  }
  { "route"
    { "address" = "111.222.123.123" }
    { "netmask" = "255.123.255.231" }
    { "gateway" = "111.222.123.1" }
    { "metric" = "5" }
  }
  { "max-routes" = "5" }
  { "route-gateway" = "gateway-name" }
  { "route-gateway" = "111.222.123.1" }
  { "route-gateway" = "dhcp" }
  { "route-metric" = "5" }
  { "route-delay" }
  { "route-delay"
    { "seconds" = "1" }
  }
  { "route-delay"
    { "seconds" = "1" }
    { "win-seconds" = "2" }
  }
  { "route-up" = "my command goes here" }
  { "route-pre-down" = "my command goes here" }
  { "route-noexec" }
  { "route-nopull" }
  { "allow-pull-fqdn" }
  { "client-nat"
    { "type" = "snat" }
    { "network" = "1.2.3.4" }
    { "netmask" = "5.6.7.8" }
    { "alias" = "9.8.7.6" }
  }
  { "client-nat"
    { "type" = "dnat" }
    { "network" = "1.2.3.4" }
    { "netmask" = "5.6.7.8" }
    { "alias" = "9.8.7.6" }
  }
  { "redirect-gateway"
    { "flag" = "local" }
  }
  { "redirect-gateway"
    { "flag" = "local" }
    { "flag" = "autolocal" }
  }
  { "redirect-gateway"
    { "flag" = "local" }
    { "flag" = "autolocal" }
    { "flag" = "def1" }
    { "flag" = "bypass-dhcp" }
    { "flag" = "bypass-dns" }
    { "flag" = "block-local" }
  }
  { "link-mtu" = "5" }
  { "redirect-private"
    { "flag" = "local" }
  }
  { "redirect-private"
    { "flag" = "local" }
    { "flag" = "autolocal" }
  }
  { "redirect-private"
    { "flag" = "local" }
    { "flag" = "autolocal" }
    { "flag" = "def1" }
    { "flag" = "bypass-dhcp" }
    { "flag" = "bypass-dns" }
    { "flag" = "block-local" }
  }
  { "tun-mtu" = "5" }
  { "tun-mtu-extra" = "5" }
  { "mtu-disc" = "no" }
  { "mtu-disc" = "maybe" }
  { "mtu-disc" = "yes" }
  { "mtu-test" }
  { "fragment" = "5" }
  { "mssfix" = "1600" }
  { "sndbuf" = "65536" }
  { "rcvbuf" = "65535" }
  { "mark" = "blahvalue" }
  { "socket-flags" = "TCP_NODELAY" }
  { "txqueuelen" = "5" }
  { "shaper" = "50" }
  { "inactive"
    { "seconds" = "5" }
  }
  { "inactive"
    { "seconds" = "5" }
    { "bytes" = "1024" }
  }
  { "ping" = "10" }
  { "ping-exit" = "10" }
  { "ping-restart" = "10" }
  { "keepalive"
    { "ping" = "1" }
    { "timeout" = "2" }
  }
  { "ping-timer-rem" }
  { "persist-tun" }
  { "persist-key" }
  { "persist-local-ip" }
  { "persist-remote-ip" }
  { "mlock" }
  { "up" = "my command goes here" }
  { "up-delay" }
  { "down" = "my command goes here" }
  { "down-pre" }
  { "up-restart" }
  { "setenv"
    { "myname" = "myvalue" }
  }
  { "setenv"
    { "my0-_name" = "my value with spaces" }
  }
  { "setenv-safe"
    { "myname" = "myvalue" }
  }
  { "setenv-safe"
    { "my-_name" = "my value with spaces" }
  }
  { "ignore-unknown-option"
    { "opt" = "anopt" }
  }
  { "ignore-unknown-option"
    { "opt" = "anopt" }
    { "opt" = "anotheropt" }
  }
  { "script-security" = "3" }
  { "disable-occ" }
  { "user" = "username" }
  { "group" = "groupname" }
  { "cd" = "/canonical/dir" }
  { "cd" = "relative/dir/" }
  { "chroot" = "/canonical/dir" }
  { "chroot" = "relative/dir/" }
  { "setcon" = "selinux-context" }
  { "daemon" }
  { "daemon" = "mydaemon_name" }
  { "syslog" }
  { "syslog" = "my_syslog-name" }
  { "errors-to-stderr" }
  { "passtos" }
  { "inetd" }
  { "inetd"
    { "mode" = "wait" }
  }
  { "inetd"
    { "mode" = "nowait" }
  }
  { "inetd"
    { "mode" = "wait" }
    { "progname" = "my-program_name" }
  }
  { "log" = "myfilename" }
  { "log-append" = "myfilename" }
  { "suppress-timestamps" }
  { "writepid" = "myfile" }
  { "nice" = "5" }
  { "fast-io" }
  { "multihome" }
  { "echo" = "stuff to echo until end of line" }
  { "remap-usr1" = "SIGHUP" }
  { "remap-usr1" = "SIGTERM" }
  { "verb" = "6" }
  { "status"
    { "file" = "myfile" }
  }
  { "status"
    { "file" = "myfile" }
    { "repeat-seconds" = "15" }
  }
  { "status-version" }
  { "status-version" = "3" }
  { "mute" = "20" }
  { "comp-lzo" }
  { "comp-lzo" = "yes" }
  { "comp-lzo" = "no" }
  { "comp-lzo" = "adaptive" }
  { "management"
    { "server" = "123.123.123.123" }
    { "port" = "1234" }
  }
  { "management"
    { "server" = "123.123.123.123" }
    { "port" = "1234" }
    { "pwfile" = "/canonical/file" }
  }
  { "management-client" }
  { "management-query-passwords" }
  { "management-query-proxy" }
  { "management-query-remote" }
  { "management-forget-disconnect" }
  { "management-hold" }
  { "management-signal" }
  { "management-up-down" }
  { "management-client-auth" }
  { "management-client-pf" }
  { "management-log-cache" = "5" }
  { "management-client-user" = "myuser" }
  { "management-client-user" = "mygroup" }
  { "plugin"
    { "file" = "/canonical/file" }
  }
  { "plugin"
    { "file" = "relative/file" }
  }
  { "plugin"
    { "file" = "myfile" }
    { "init-string" = "an init string" }
  }
  { "server"
    { "address" = "1.2.3.4" }
    { "netmask" = "255.255.255.0" }
  }
  { "server"
    { "address" = "1.2.3.4" }
    { "netmask" = "255.255.255.255" }
    { "nopool" }
  }
  { "server-bridge"
    { "address" = "1.2.3.4" }
    { "netmask" = "1.2.3.5" }
    { "start" = "50.5.5.5" }
    { "end" = "50.5.5.6" }
  }
  { "server-bridge" = "nogw" }
  { "push" = "my push string" }
  { "push-reset" }
  { "push-peer-info" }
  { "disable" }
  { "ifconfig-pool"
    { "start" = "1.1.1.1" }
    { "end" = "2.2.2.2" }
  }
  { "ifconfig-pool"
    { "start" = "1.1.1.1" }
    { "end" = "2.2.2.2" }
    { "netmask" = "255.255.255.0" }
  }
  { "ifconfig-pool-persist"
    { "file" = "myfile" }
  }
  { "ifconfig-pool-persist"
    { "file" = "myfile" }
    { "seconds" = "50" }
  }
  { "ifconfig-pool-linear" }
  { "ifconfig-push"
    { "local" = "1.1.1.1" }
    { "remote-netmask" = "2.2.2.2" }
  }
  { "ifconfig-push"
    { "local" = "1.1.1.1" }
    { "remote-netmask" = "2.2.2.2" }
    { "alias" = "alias-name" }
  }
  { "iroute"
    { "local" = "1.1.1.1" }
  }
  { "iroute"
    { "local" = "1.1.1.1" }
    { "netmask" = "2.2.2.2" }
  }
  { "client-to-client" }
  { "duplicate-cn" }
  { "client-connect" = "my command goes here" }
  { "client-disconnect" = "my command goes here" }
  { "client-config-dir" = "directory" }
  { "ccd-exclusive" }
  { "tmp-dir" = "/directory" }
  { "hash-size"
    { "real" = "1" }
    { "virtual" = "2" }
  }
  { "bcast-buffers" = "5" }
  { "tcp-queue-limit" = "50" }
  { "tcp-nodelay" }
  { "max-clients" = "50" }
  { "max-routes-per-client" = "50" }
  { "stale-routes-check"
    { "age" = "5" }
  }
  { "stale-routes-check"
    { "age" = "5" }
    { "interval" = "50" }
  }
  { "connect-freq"
    { "num" = "50" }
    { "sec" = "100" }
  }
  { "learn-address" = "my command goes here" }
  { "auth-user-pass-verify"
    { 
      { "command" = "/my/script/with/no/arguments.sh" }
    }
    { "method" = "via-env" }
  }
  { "auth-user-pass-verify"
    { 
      { "command" = "myscript.sh arg1 arg2" }
    }
    { "method" = "via-file" }
  }
  { "opt-verify" }
  { "auth-user-pass-optional" }
  { "client-cert-not-required" }
  { "username-as-common-name" }
  { "port-share"
    { "host" = "1.1.1.1" }
    { "port" = "1234" }
  }
  { "port-share"
    { "host" = "myhostname" }
    { "port" = "1234" }
  }
  { "port-share"
    { "host" = "myhostname" }
    { "port" = "1234" }
    { "dir" = "/canonical/dir" }
  }
  { "client" }
  { "pull" }
  { "auth-user-pass" }
  { "auth-user-pass" = "/canonical/file" }
  { "auth-user-pass" = "relative/file" }
  { "auth-retry" = "none" }
  { "auth-retry" = "nointeract" }
  { "auth-retry" = "interact" }
  { "static-challenge"
    {
      { "text" = "challenge_no_spaces" }
    }
    { "echo" = "1" }
  }
  { "static-challenge"
    {
      { "text" = "my quoted challenge string" }
    }
    { "echo" = "0" }
  }
  { "server-poll-timeout" = "50" }
  { "explicit-exit-notify" }
  { "explicit-exit-notify" = "5" }
  { "secret"
    { "file" = "/canonicalfile" }
  }
  { "secret"
    { "file" = "relativefile" }
  }
  { "secret"
    { "file" = "filename" }
    { "direction" = "1" }
  }
  { "secret"
    { "file" = "filename" }
    { "direction" = "0" }
  }
  { "key-direction" }
  { "auth" = "none" }
  { "auth" = "sha1" }
  { "cipher" = "SHA1" }
  { "cipher" = "sha1" }
  { "keysize" = "50" }
  { "prng"
    { "algorithm" = "SHA1" }
  }
  { "prng"
    { "algorithm" = "SHA1" }
    { "nsl" = "500" }
  }
  { "engine" }
  { "engine" = "blah" }
  { "no-replay" }
  { "replay-window"
    { "window-size" = "64" }
  }
  { "replay-window"
    { "window-size" = "64" }
    { "seconds" = "16" }
  }
  { "mute-replay-warnings" }
  { "replay-persist" = "/my/canonical/filename" }
  { "no-iv" }
  { "use-prediction-resistance" }
  { "test-crypto" }
  { "tls-server" }
  { "tls-client" }
  { "ca" = "myfile" }
  { "capath" = "/mydir/" }
  { "dh" = "myfile" }
  { "cert" = "myfile" }
  { "extra-certs" = "myfile" }
  { "key" = "myfile" }
  { "tls-version-min" = "1.1" }
  { "tls-version-min" = "2" }
  { "tls-version-min" = "1.1"
    { "or-highest" }
  }
  { "tls-version-max" = "5.5" }
  { "pkcs12" = "myfile" }
  { "verify-hash" = "AD:B0:95:D8:09:C8:36:45:12:A9:89:C8:90:09:CB:13:72:A6:AD:16" }
  { "pkcs11-cert-private" = "0" }
  { "pkcs11-cert-private" = "1" }
  { "pkcs11-id" = "myname" }
  { "pkcs11-id-management" }
  { "pkcs11-pin-cache" = "50" }
  { "pkcs11-protected-authentication" = "0" }
  { "pkcs11-protected-authentication" = "1" }
  { "cryptoapicert"
    { "SUBJ" = "Justin Akers" }
  }
  { "key-method" = "2" }
  { "tls-cipher"
    { "cipher" = "DEFAULT" }
    { "cipher" = "!EXP" }
    { "cipher" = "!PSK" }
    { "cipher" = "!SRP" }
    { "cipher" = "!kRSA" }
  }
  { "tls-timeout" = "50" }
  { "reneg-bytes" = "50" }
  { "reneg-pkts" = "50" }
  { "reneg-sec" = "5" }
  { "hand-window" = "123" }
  { "tran-window" = "456" }
  { "single-session" }
  { "tls-exit" }
  { "tls-auth"
    { "key" = "filename" }
    { "is_client" = "1" }
  }
  { "askpass" = "/canonical/filename" }
  { "auth-nocache" }
  { "tls-verify" = "my command goes here" }
  { "tls-export-cert" = "/a/directory/for/things" }
  { "x509-username-field"
    { "subj" = "emailAddress" }
  }
  { "x509-username-field"
    { "ext" = "subjectAltName" }
  }
  { "tls-remote" = "myhostname" }
  { "verify-x509-name"
    { "name" = "hostname" }
    { "type" = "name" }
  }
  { "verify-x509-name"
    { "name" = "hostname" }
    { "type" = "name-prefix" }
  }
  { "verify-x509-name"
    { "name" = "hostname" }
    { "type" = "subject" }
  }
  { "ns-cert-type" = "server" }
  { "ns-cert-type" = "client" }
  { "remote-cert-tls" = "server" }
  { "remote-cert-tls" = "client" }
  { "remote-cert-ku"
    { "usage" = "01" }
  }
  { "remote-cert-ku"
    { "usage" = "01" }
    { "usage" = "02" }
    { "usage" = "fa" }
    { "usage" = "FF" }
    { "usage" = "b3" }
  }
  { "remote-cert-eku"
    { "oid" = "123.3510.350.10" }
  }
  { "remote-cert-eku"
    { "symbol" = "TLS Web Client Authentication" }
  }
  { "remote-cert-eku"
    { "symbol" = "serverAuth" }
  }
  { "crl-verify" = "/a/file/path" }
  { "crl-verify" = "/a/directory/"
    { "dir" }
  }
  { "show-ciphers" }
  { "show-digests" }
  { "show-tls" }
  { "show-engines" }
  { "genkey" }
  { "mktun" }
  { "rmtun" }
  { "ifconfig-ipv6"
    { "address" = "2000:123:456::/64" }
    { "remote" = "1234:99:123::124" }
  }
  { "ifconfig-ipv6-push"
    { "address" = "2000:123:456::/64" }
    { "remote" = "1234:99:123::124" }
  }
  { "iroute-ipv6" = "2000:123:456::/64" }
  { "route-ipv6"
    { "network" = "2000:123:456::/64" }
  }
  { "route-ipv6"
    { "network" = "2000:123:456::/64" }
    { "gateway" = "1234:99:123::124" }
  }
  { "route-ipv6"
    { "network" = "2000:123:456::/64" }
    { "gateway" = "1234:99:123::124" }
    { "metric" = "500" }
  }
  { "server-ipv6" = "2000:123:456::/64" }
  { "ifconfig-ipv6-pool" = "2000:123:456::/64" }
  {  }


