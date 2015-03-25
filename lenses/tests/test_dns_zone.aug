module Test_Dns_Zone =

let lns = Dns_Zone.lns

(* RFC 1034 §6 *)
test lns get "
EDU.  IN SOA SRI-NIC.ARPA. HOSTMASTER.SRI-NIC.ARPA. (
                        870729 ;serial
                        1800 ;refresh every 30 minutes
                        300 ;retry every 5 minutes
                        604800 ;expire after a week
                        86400 ;minimum of a day
                        )
                NS SRI-NIC.ARPA.
                NS C.ISI.EDU.

UCI 172800 NS ICS.UCI
                172800 NS ROME.UCI
ICS.UCI 172800 A 192.5.19.1
ROME.UCI 172800 A 192.5.19.31

ISI 172800 NS VAXA.ISI
                172800 NS A.ISI
                172800 NS VENERA.ISI.EDU.
VAXA.ISI 172800 A 10.2.0.27
                172800 A 128.9.0.33
VENERA.ISI.EDU. 172800 A 10.1.0.52
                172800 A 128.9.0.32
A.ISI 172800 A 26.3.0.103

UDEL.EDU.  172800 NS LOUIE.UDEL.EDU.
                172800 NS UMN-REI-UC.ARPA.
LOUIE.UDEL.EDU. 172800 A 10.0.0.96
                172800 A 192.5.39.3

YALE.EDU.  172800 NS YALE.ARPA.
YALE.EDU.  172800 NS YALE-BULLDOG.ARPA.

MIT.EDU.  43200 NS XX.LCS.MIT.EDU.
                  43200 NS ACHILLES.MIT.EDU.
XX.LCS.MIT.EDU.  43200 A 10.0.0.44
ACHILLES.MIT.EDU. 43200 A 18.72.0.8
" =
  { "EDU."
    { "1"
      { "class" = "IN" }
      { "type" = "SOA" }
      { "mname" = "SRI-NIC.ARPA." }
      { "rname" = "HOSTMASTER.SRI-NIC.ARPA." }
      { "serial" = "870729" }
      { "refresh" = "1800" }
      { "retry" = "300" }
      { "expiry" = "604800" }
      { "minimum" = "86400" }
    }
    { "2" { "type" = "NS" } { "rdata" = "SRI-NIC.ARPA." } }
    { "3" { "type" = "NS" } { "rdata" = "C.ISI.EDU." } }
  }
  { "UCI"
    { "1" { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "ICS.UCI" } }
    { "2" { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "ROME.UCI" } }
  }
  { "ICS.UCI"
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "192.5.19.1" } }
  }
  { "ROME.UCI"
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "192.5.19.31" } }
  }
  { "ISI"
    { "1" { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "VAXA.ISI" } }
    { "2" { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "A.ISI" } }
    { "3"
      { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "VENERA.ISI.EDU." }
    }
  }
  { "VAXA.ISI"
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "10.2.0.27" } }
    { "2" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "128.9.0.33" } }
  }
  { "VENERA.ISI.EDU."
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "10.1.0.52" } }
    { "2" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "128.9.0.32" } }
  }
  { "A.ISI"
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "26.3.0.103" } }
  }
  { "UDEL.EDU."
    { "1"
      { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "LOUIE.UDEL.EDU." }
    }
    { "2"
      { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "UMN-REI-UC.ARPA." }
    }
  }
  { "LOUIE.UDEL.EDU."
    { "1" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "10.0.0.96" } }
    { "2" { "ttl" = "172800" } { "type" = "A" } { "rdata" = "192.5.39.3" } }
  }
  { "YALE.EDU."
    { "1" { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "YALE.ARPA." } }
  }
  { "YALE.EDU."
    { "1"
      { "ttl" = "172800" } { "type" = "NS" } { "rdata" = "YALE-BULLDOG.ARPA." }
    }
  }
  { "MIT.EDU."
    { "1"
      { "ttl" = "43200" } { "type" = "NS" } { "rdata" = "XX.LCS.MIT.EDU." }
    }
    { "2"
      { "ttl" = "43200" } { "type" = "NS" } { "rdata" = "ACHILLES.MIT.EDU." }
    }
  }
  { "XX.LCS.MIT.EDU."
    { "1" { "ttl" = "43200" } { "type" = "A" } { "rdata" = "10.0.0.44" } }
  }
  { "ACHILLES.MIT.EDU."
    { "1" { "ttl" = "43200" } { "type" = "A" } { "rdata" = "18.72.0.8" } }
  }


(* RFC 1035 §5.3 *)
test lns get "
@   IN  SOA     VENERA      Action\.domains (
                                 20     ; SERIAL
                                 7200   ; REFRESH
                                 600    ; RETRY
                                 3600000; EXPIRE
                                 60)    ; MINIMUM

        NS      A.ISI.EDU.
        NS      VENERA
        NS      VAXA
        MX      10      VENERA
        MX      20      VAXA

A       A       26.3.0.103

VENERA  A       10.1.0.52
        A       128.9.0.32

VAXA    A       10.2.0.27
        A       128.9.0.33
" =
  { "@"
    { "1"
      { "class" = "IN" }
      { "type" = "SOA" }
      { "mname" = "VENERA" }
      { "rname" = "Action\\.domains" }
      { "serial" = "20" }
      { "refresh" = "7200" }
      { "retry" = "600" }
      { "expiry" = "3600000" }
      { "minimum" = "60" }
    }
    { "2" { "type" = "NS" } { "rdata" = "A.ISI.EDU." } }
    { "3" { "type" = "NS" } { "rdata" = "VENERA" } }
    { "4" { "type" = "NS" } { "rdata" = "VAXA" } }
    { "5" { "type" = "MX" } { "priority" = "10" } { "exchange" = "VENERA" } }
    { "6" { "type" = "MX" } { "priority" = "20" } { "exchange" = "VAXA" } }
  }
  { "A" { "1" { "type" = "A" } { "rdata" = "26.3.0.103" } } }
  { "VENERA"
    { "1" { "type" = "A" } { "rdata" = "10.1.0.52" } }
    { "2" { "type" = "A" } { "rdata" = "128.9.0.32" } }
  }
  { "VAXA"
    { "1" { "type" = "A" } { "rdata" = "10.2.0.27" } }
    { "2" { "type" = "A" } { "rdata" = "128.9.0.33" } }
  }


(* RFC 2782 *)
test lns get "
$ORIGIN example.com.
@               SOA server.example.com. root.example.com. (
                    1995032001 3600 3600 604800 86400 )
                NS  server.example.com.
                NS  ns1.ip-provider.net.
                NS  ns2.ip-provider.net.
; foobar - use old-slow-box or new-fast-box if either is
; available, make three quarters of the logins go to
; new-fast-box.
_foobar._tcp    SRV 0 1 9 old-slow-box.example.com.
                 SRV 0 3 9 new-fast-box.example.com.
; if neither old-slow-box or new-fast-box is up, switch to
; using the sysdmin's box and the server
                 SRV 1 0 9 sysadmins-box.example.com.
                 SRV 1 0 9 server.example.com.
server           A   172.30.79.10
old-slow-box     A   172.30.79.11
sysadmins-box    A   172.30.79.12
new-fast-box     A   172.30.79.13
; NO other services are supported
*._tcp          SRV  0 0 0 .
*._udp          SRV  0 0 0 .
" =
  { "$ORIGIN" = "example.com." }
  { "@"
    { "1"
      { "type" = "SOA" }
      { "mname" = "server.example.com." }
      { "rname" = "root.example.com." }
      { "serial" = "1995032001" }
      { "refresh" = "3600" }
      { "retry" = "3600" }
      { "expiry" = "604800" }
      { "minimum" = "86400" }
    }
    { "2" { "type" = "NS" } { "rdata" = "server.example.com." } }
    { "3" { "type" = "NS" } { "rdata" = "ns1.ip-provider.net." } }
    { "4" { "type" = "NS" } { "rdata" = "ns2.ip-provider.net." } }
  }
  { "_foobar._tcp"
    { "1"
      { "type" = "SRV" }
      { "priority" = "0" }
      { "weight" = "1" }
      { "port" = "9" }
      { "target" = "old-slow-box.example.com." }
    }
    { "2"
      { "type" = "SRV" }
      { "priority" = "0" }
      { "weight" = "3" }
      { "port" = "9" }
      { "target" = "new-fast-box.example.com." }
    }
    { "3"
      { "type" = "SRV" }
      { "priority" = "1" }
      { "weight" = "0" }
      { "port" = "9" }
      { "target" = "sysadmins-box.example.com." }
    }
    { "4"
      { "type" = "SRV" }
      { "priority" = "1" }
      { "weight" = "0" }
      { "port" = "9" }
      { "target" = "server.example.com." }
    }
  }
  { "server" { "1" { "type" = "A" } { "rdata" = "172.30.79.10" } } }
  { "old-slow-box" { "1" { "type" = "A" } { "rdata" = "172.30.79.11" } } }
  { "sysadmins-box" { "1" { "type" = "A" } { "rdata" = "172.30.79.12" } } }
  { "new-fast-box" { "1" { "type" = "A" } { "rdata" = "172.30.79.13" } } }
  { "*._tcp"
    { "1"
      { "type" = "SRV" }
      { "priority" = "0" }
      { "weight" = "0" }
      { "port" = "0" }
      { "target" = "." }
    }
  }
  { "*._udp"
    { "1"
      { "type" = "SRV" }
      { "priority" = "0" }
      { "weight" = "0" }
      { "port" = "0" }
      { "target" = "." }
    }
  }


(* RFC 3403 §6.2 *)
test lns get "
$ORIGIN 2.1.2.1.5.5.5.0.7.7.1.e164.arpa.
 IN NAPTR 100 10 \"u\" \"sip+E2U\"  \"!^.*$!sip:information@foo.se!i\"     .
 IN NAPTR 102 10 \"u\" \"smtp+E2U\" \"!^.*$!mailto:information@foo.se!i\"  .
" =
  { "$ORIGIN" = "2.1.2.1.5.5.5.0.7.7.1.e164.arpa." }
  { "@"
    { "1"
      { "class" = "IN" }
      { "type" = "NAPTR" }
      { "order" = "100" }
      { "preference" = "10" }
      { "flags" = "\"u\"" }
      { "service" = "\"sip+E2U\"" }
      { "regexp" = "\"!^.*$!sip:information@foo.se!i\"" }
      { "replacement" = "." }
    }
    { "2"
      { "class" = "IN" }
      { "type" = "NAPTR" }
      { "order" = "102" }
      { "preference" = "10" }
      { "flags" = "\"u\"" }
      { "service" = "\"smtp+E2U\"" }
      { "regexp" = "\"!^.*$!mailto:information@foo.se!i\"" }
      { "replacement" = "." }
    }
  }


(* SOA record on a single line *)
test lns get "
$ORIGIN example.com.
@ IN SOA ns root.example.com. (1 2 3 4 5)
" =
  { "$ORIGIN" = "example.com." }
  { "@"
    { "1"
      { "class" = "IN" }
      { "type" = "SOA" }
      { "mname" = "ns" }
      { "rname" = "root.example.com." }
      { "serial" = "1" }
      { "refresh" = "2" }
      { "retry" = "3" }
      { "expiry" = "4" }
      { "minimum" = "5" }
    }
  }


(* Different ordering of TTL and class *)
test lns get "
$ORIGIN example.com.
foo 1D IN A 10.1.2.3
bar IN 2W A 10.4.5.6
" =
  { "$ORIGIN" = "example.com." }
  { "foo"
    { "1"
      { "ttl" = "1D" }
      { "class" = "IN" }
      { "type" = "A" }
      { "rdata" = "10.1.2.3" }
    }
  }
  { "bar"
    { "1"
      { "class" = "IN" }
      { "ttl" = "2W" }
      { "type" = "A" }
      { "rdata" = "10.4.5.6" }
    }
  }


(* Escaping *)
test lns get "
$ORIGIN example.com.
foo TXT abc\\\\def\\\"ghi
bar TXT \"ab cd\\\\ef\\\"gh\"
" =
  { "$ORIGIN" = "example.com." }
  { "foo" { "1" { "type" = "TXT" } { "rdata" = "abc\\\\def\\\"ghi" } } }
  { "bar" { "1" { "type" = "TXT" } { "rdata" = "\"ab cd\\\\ef\\\"gh\"" } } }


(* Whitespace at the end of the line *)
test lns get "
$ORIGIN example.com. \n@ IN SOA ns root.example.com. (1 2 3 4 5) \t
foo 1D IN A 10.1.2.3\t
" =
  { "$ORIGIN" = "example.com." }
  { "@"
    { "1"
      { "class" = "IN" }
      { "type" = "SOA" }
      { "mname" = "ns" }
      { "rname" = "root.example.com." }
      { "serial" = "1" }
      { "refresh" = "2" }
      { "retry" = "3" }
      { "expiry" = "4" }
      { "minimum" = "5" }
    }
  }
  { "foo"
    { "1"
      { "ttl" = "1D" }
      { "class" = "IN" }
      { "type" = "A" }
      { "rdata" = "10.1.2.3" }
    }
  }
