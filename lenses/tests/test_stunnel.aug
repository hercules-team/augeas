module Test_stunnel =
    let conf ="; Test stunnel-like config file
; Foo bar baz
cert = /path/1
key = /path/2

sslVersion = SSLv3

; another comment

[service1]
accept = 49999
connect = servicedest:1234

[service2]
accept = 1234
"

    test Stunnel.lns get conf =
        { ".anon"
            { "#comment" = "Test stunnel-like config file" }
            { "#comment" = "Foo bar baz" }
            { "cert" = "/path/1" }
            { "key" = "/path/2" }
            {}
            { "sslVersion" = "SSLv3" }
            {}
            { "#comment" = "another comment" }
            {}
        }
        { "service1"
            { "accept" = "49999" }
            { "connect" = "servicedest:1234" }
            {}
        }
        { "service2"
            { "accept" = "1234" }
        }
