module Test_rx =

let sto_ipv4 = [ label "IP" . store Rx.ipv4 ]

test sto_ipv4 get "192.168.0.1"     = { "IP" = "192.168.0.1"     }
test sto_ipv4 get "255.255.255.254" = { "IP" = "255.255.255.254" }

let sto_ipv6 = [ label "IP" . store Rx.ipv6 ]
test sto_ipv6 get "fe80::215:f2ff:fea4:b8d9" = { "IP" = "fe80::215:f2ff:fea4:b8d9" }

let sto_ip = [ label "IP" . store Rx.ip ]

test sto_ip get "192.168.0.1"              = { "IP" = "192.168.0.1"              }
test sto_ip get "255.255.255.254"          = { "IP" = "255.255.255.254"          }
test sto_ip get "fe80::215:f2ff:fea4:b8d9" = { "IP" = "fe80::215:f2ff:fea4:b8d9" }
