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

(* iso_8601 *)
let iso_8601 = [ label "date" . store Rx.iso_8601 ]

test iso_8601 get "2009-12T12:34" = { "date" = "2009-12T12:34" }
test iso_8601 get "2009" = { "date" = "2009" }
test iso_8601 get "2009-05-19" = { "date" = "2009-05-19" }
test iso_8601 get "2009-05-19" = { "date" = "2009-05-19" }
test iso_8601 get "20090519" = { "date" = "20090519" }
test iso_8601 get "2009123" = { "date" = "2009123" }
test iso_8601 get "2009-05" = { "date" = "2009-05" }
test iso_8601 get "2009-123" = { "date" = "2009-123" }
test iso_8601 get "2009-222" = { "date" = "2009-222" }
test iso_8601 get "2009-001" = { "date" = "2009-001" }
test iso_8601 get "2009-W01-1" = { "date" = "2009-W01-1" }
test iso_8601 get "2009-W51-1" = { "date" = "2009-W51-1" }
test iso_8601 get "2009-W511" = { "date" = "2009-W511" }
test iso_8601 get "2009-W33" = { "date" = "2009-W33" }
test iso_8601 get "2009W511" = { "date" = "2009W511" }
test iso_8601 get "2009-05-19" = { "date" = "2009-05-19" }
test iso_8601 get "2009-05-19 00:00" = { "date" = "2009-05-19 00:00" }
test iso_8601 get "2009-05-19 14" = { "date" = "2009-05-19 14" }
test iso_8601 get "2009-05-19 14:31" = { "date" = "2009-05-19 14:31" }
test iso_8601 get "2009-05-19 14:39:22" = { "date" = "2009-05-19 14:39:22" }
test iso_8601 get "2009-05-19T14:39Z" = { "date" = "2009-05-19T14:39Z" }
test iso_8601 get "2009-W21-2" = { "date" = "2009-W21-2" }
test iso_8601 get "2009-W21-2T01:22" = { "date" = "2009-W21-2T01:22" }
test iso_8601 get "2009-139" = { "date" = "2009-139" }
test iso_8601 get "2009-05-19 14:39:22-06:00" = { "date" = "2009-05-19 14:39:22-06:00" }
test iso_8601 get "2009-05-19 14:39:22+0600" = { "date" = "2009-05-19 14:39:22+0600" }
test iso_8601 get "2009-05-19 14:39:22-01" = { "date" = "2009-05-19 14:39:22-01" }
test iso_8601 get "20090621T0545Z" = { "date" = "20090621T0545Z" }
test iso_8601 get "2007-04-06T00:00" = { "date" = "2007-04-06T00:00" }
test iso_8601 get "2007-04-05T24:00" = { "date" = "2007-04-05T24:00" }
test iso_8601 get "2010-02-18T16:23:48.5" = { "date" = "2010-02-18T16:23:48.5" }
test iso_8601 get "2010-02-18T16:23:48,444" = { "date" = "2010-02-18T16:23:48,444" }
test iso_8601 get "2010-02-18T16:23:48,3-06:00" = { "date" = "2010-02-18T16:23:48,3-06:00" }
test iso_8601 get "2010-02-18T16:23.4" = { "date" = "2010-02-18T16:23.4" }
test iso_8601 get "2010-02-18T16:23,25" = { "date" = "2010-02-18T16:23,25" }
test iso_8601 get "2010-02-18T16:23.33+0600" = { "date" = "2010-02-18T16:23.33+0600" }
test iso_8601 get "2010-02-18T16.23334444" = { "date" = "2010-02-18T16.23334444" }
test iso_8601 get "2010-02-18T16,2283" = { "date" = "2010-02-18T16,2283" }
test iso_8601 get "2009-05-19 143922.500" = { "date" = "2009-05-19 143922.500" }
test iso_8601 get "2009-05-19 1439,55" = { "date" = "2009-05-19 1439,55" }

(* url_3986 *)
let url_3986 = [ label "url" . store Rx.url_3986 ]

test url_3986 get "http://tools.ietf.org/rfc/rfc3986.txt" = { "url" =  "http://tools.ietf.org/rfc/rfc3986.txt" }
test url_3986 get "https://github.com/hercules-team/augeas/" = { "url" = "https://github.com/hercules-team/augeas/" }
test url_3986 get "http://www.ics.uci.edu:80/pub/ietf/uri/#Related" = { "url" =  "http://www.ics.uci.edu:80/pub/ietf/uri/#Related" }
test url_3986 get "EXAMPLE://a/./b/../b/%63/%7bfoo%7d" = { "url" =  "EXAMPLE://a/./b/../b/%63/%7bfoo%7d" }
test url_3986 get "http://a/b/c/g;?x=1/y#z" = { "url" =  "http://a/b/c/g;?x=1/y#z" }
test url_3986 get "eXaMpLe://a.very.sub.domain.tld:1234/b/c/e/f/g.txt;?x=1/y&q=%7b-w-%7b#z" = { "url" = "eXaMpLe://a.very.sub.domain.tld:1234/b/c/e/f/g.txt;?x=1/y&q=%7b-w-%7b#z" }

