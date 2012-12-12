module Test_pg_hba =

    (* Main test *)
    let conf ="# TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD

local   all         all                               ident sameuser
# IPv4 local connections:
host    all         all         127.0.0.1/32          md5
# Remote connections by hostname:
host    all         all         foo.example.com       md5
# Remote connections by suffix of hostname/fqdn:
host    all         all         .example.com          md5
# IPv6 local connections:
host    all         all         ::1/128               md5
"

    test Pg_Hba.lns get conf =
        { "#comment" = "TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD" }
        {}
        { "1"
            { "type" = "local" }
            { "database" = "all" }
            { "user"  = "all" }
            { "method" = "ident"
                { "option" = "sameuser" } }
        }
        { "#comment" = "IPv4 local connections:" }
        { "2"
            { "type" = "host" }
            { "database" = "all" }
            { "user"  = "all" }
            { "address" = "127.0.0.1/32" }
            { "method" = "md5" }
        }
        { "#comment" = "Remote connections by hostname:" }
        { "3"
            { "type" = "host" }
            { "database" = "all" }
            { "user"  = "all" }
            { "address" = "foo.example.com" }
            { "method" = "md5" }
        }
        { "#comment" = "Remote connections by suffix of hostname/fqdn:" }
        { "4"
            { "type" = "host" }
            { "database" = "all" }
            { "user"  = "all" }
            { "address" = ".example.com" }
            { "method" = "md5" }
        }
        { "#comment" = "IPv6 local connections:" }
        { "5"
            { "type" = "host" }
            { "database" = "all" }
            { "user"  = "all" }
            { "address" = "::1/128" }
            { "method" = "md5" }
        }

(* ------------------------------------------------------------- *)

    (* Simple local test *)
    test Pg_Hba.lns get "local all all trust\n" =
        { "1"
            { "type" = "local" }
            { "database" = "all" }
            { "user"  = "all" }
            { "method" = "trust" }
        }

    (* Remote test with comma-sparated database names *)
    test Pg_Hba.lns get "hostssl db1,db2,db3 +pgusers 127.0.0.1/32 trust\n" =
        { "1"
            { "type" = "hostssl" }
            { "database" = "db1" }
            { "database" = "db2" }
            { "database" = "db3" }
            { "user"  = "+pgusers" }
            { "address" = "127.0.0.1/32" }
            { "method" = "trust" }
        }

    (* Test with comma-sparated user names *)
    test Pg_Hba.lns get "hostnossl sameuser u1,u2,u3 127.0.0.1/32 trust\n" =
        { "1"
            { "type" = "hostnossl" }
            { "database" = "sameuser" }
            { "user"  = "u1" }
            { "user"  = "u2" }
            { "user"  = "u3" }
            { "address" = "127.0.0.1/32" }
            { "method" = "trust" }
        }

    (* Test with quoted database and user names *)
    test Pg_Hba.lns get "host \"sameuser\" \"all\" 127.0.0.1/32 trust\n" =
        { "1"
            { "type" = "host" }
            { "database" = "\"sameuser\"" }
            { "user"  = "\"all\"" }
            { "address" = "127.0.0.1/32" }
            { "method" = "trust" }
        }

    (* Test with IP + netmask address format *)
    test Pg_Hba.lns get "host all all 192.168.1.1 255.255.0.0 trust\n" =
        { "1"
            { "type" = "host" }
            { "database" = "all" }
            { "user"  = "all" }
            { "address" = "192.168.1.1 255.255.0.0" }
            { "method" = "trust" }
        }
    
    (* Test with fqdn as address *)
    test Pg_Hba.lns get "host all all foo.example.com md5\n" = 
        { "1"
            { "type" = "host" }
            { "database" = "all" }
            { "user" = "all" }
            { "address" = "foo.example.com" }
            { "method" = "md5" }
        }

    (* Test with fqdn suffix as address *)
    test Pg_Hba.lns get "host all all .example.com md5\n" = 
        { "1"
            { "type" = "host" }
            { "database" = "all" }
            { "user" = "all" }
            { "address" = ".example.com" }
            { "method" = "md5" }
        }

    (* Local types may not have and address *)
    test Pg_Hba.lns get "local all all 127.0.0.1/32 trust\n" = *

    (* Remote types must have an address *)
    test Pg_Hba.lns get "host all all trust\n" = *

    (* The space between the IP and the netmask must not be considered as a
       column separator ("method" is missing here) *)
    test Pg_Hba.lns get "host all all 192.168.1.1 255.255.0.0\n" = *

    (* Ticket #313: support authentication method options *)
    test Pg_Hba.lns get "host all all .dev.example.com gss include_realm=0 krb_realm=EXAMPLE.COM map=somemap
host all all .dev.example.com ldap ldapserver=auth.example.com ldaptls=1 ldapprefix=\"uid=\" ldapsuffix=\",ou=people,dc=example,dc=com\"\n" =
        { "1"
            { "type" = "host" }
            { "database" = "all" }
            { "user" = "all" }
            { "address" = ".dev.example.com" }
            { "method" = "gss"
              { "option" = "include_realm"
                { "value" = "0" } }
              { "option" = "krb_realm"
                { "value" = "EXAMPLE.COM" } }
              { "option" = "map"
                { "value" = "somemap" } } } }
        { "2"
            { "type" = "host" }
            { "database" = "all" }
            { "user" = "all" }
            { "address" = ".dev.example.com" }
            { "method" = "ldap"
              { "option" = "ldapserver"
                { "value" = "auth.example.com" } }
              { "option" = "ldaptls"
                { "value" = "1" } }
              { "option" = "ldapprefix"
                { "value" = "uid=" } }
              { "option" = "ldapsuffix"
                { "value" = ",ou=people,dc=example,dc=com" } } } }

    (* Unsupported yet *)
    (* test Pg_Hba.lns get "host \"db with spaces\" \"user with spaces\" 127.0.0.1/32 trust\n" =? *)
    (* test Pg_Hba.lns get "host \"db,with,commas\" \"user,with,commas\" 127.0.0.1/32 trust\n" =? *)
