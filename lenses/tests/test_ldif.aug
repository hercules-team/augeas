(* Test for LDIF lens *)
module Test_ldif =

  (* Test LDIF content only *)
  let content = "version: 1
dn: cn=foo bar,dc=example,dc=com
# test
ou: example value
cn:: Zm9vIGJhcg==
# test
telephoneNumber;foo;bar: +1 123 456 789
binary;foo:< file:///file/something
# test

dn: cn=simple,dc=example,dc=com
cn: simple
test: split line starts with
  :colon

dn:: Y249c2ltcGxlLGRjPWV4YW1wbGUsZGM9Y29t
# test
cn: simple

dn: cn=simple,dc=exam
    ple,dc=com
cn: simple
telephoneNumber:: KzEgMTIzIDQ1
  NiA3ODk=

# test
"

  test Ldif.lns get content =
    { "@content"
        { "version" = "1" }
        { "1" = "cn=foo bar,dc=example,dc=com"
            { "#comment" = "test" }
            { "ou" = "example value" }
            { "cn"
                { "@base64" = "Zm9vIGJhcg==" } }
            { "#comment" = "test" }
            { "telephoneNumber" = "+1 123 456 789"
                { "@option" = "foo" }
                { "@option" = "bar" } }
            { "binary"
                { "@option" = "foo" }
                { "@url" = "file:///file/something" } } }
        { "#comment" = "test" }
        {}
        { "2" = "cn=simple,dc=example,dc=com"
            { "cn" = "simple" }
            { "test" = "split line starts with
  :colon" } }
        {}
        { "3"
            { "@base64" = "Y249c2ltcGxlLGRjPWV4YW1wbGUsZGM9Y29t" }
            { "#comment" = "test" }
            { "cn" = "simple" } }
        {}
        { "4" = "cn=simple,dc=exam
    ple,dc=com"
            { "cn" = "simple" }
            { "telephoneNumber"
                { "@base64" = "KzEgMTIzIDQ1
  NiA3ODk=" } } }
        {}
        { "#comment" = "test" }
    }

  (* Test LDIF changes *)
  let changes = "version: 1
dn: cn=foo,dc=example,dc=com
changetype: delete

dn: cn=simple,dc=example,dc=com
control: 1.2.3.4
control: 1.2.3.4  true
# test
control: 1.2.3.4  true: foo bar
control: 1.2.3.4  true:: Zm9vIGJhcg==
changetype: add
cn: simple

dn: cn=foo bar,dc=example,dc=com
changeType: modify
add: telephoneNumber
telephoneNumber: +1 123 456 789
-
replace: homePostalAddress;lang-fr
homePostalAddress;lang-fr: 34 rue de Seine
# test
-
delete: telephoneNumber
-
replace: telephoneNumber
telephoneNumber:: KzEgMTIzIDQ1NiA3ODk=
-

dn: cn=foo,dc=example,dc=com
changetype: moddn
newrdn: cn=bar
deleteoldrdn: 0
newsuperior: dc=example,dc=net
"

  test Ldif.lns get changes =
    { "@changes"
        { "version" = "1" }
        { "1" = "cn=foo,dc=example,dc=com"
            { "changetype" = "delete" } }
        {}
        { "2" = "cn=simple,dc=example,dc=com"
            { "control" = "1.2.3.4" }
            { "control" = "1.2.3.4"
                { "criticality" = "true" } }
            { "#comment" = "test" }
            { "control" = "1.2.3.4"
                { "criticality" = "true" }
                { "value" = "foo bar" } }
            { "control" = "1.2.3.4"
                { "criticality" = "true" }
                { "value"
                    { "@base64" = "Zm9vIGJhcg==" } } }
            { "changetype" = "add" }
            { "cn" = "simple" } }
        {}
        { "3" = "cn=foo bar,dc=example,dc=com"
            { "changeType" = "modify" }
            { "add" = "telephoneNumber"
                { "telephoneNumber" = "+1 123 456 789" } }
            { "replace" = "homePostalAddress"
                { "@option" = "lang-fr" }
                { "homePostalAddress" = "34 rue de Seine"
                    { "@option" = "lang-fr" } }
                { "#comment" = "test" } }
            { "delete" = "telephoneNumber" }
            { "replace" = "telephoneNumber"
                { "telephoneNumber"
                    { "@base64" = "KzEgMTIzIDQ1NiA3ODk=" } } } }
        {}
        { "4" = "cn=foo,dc=example,dc=com"
            { "changetype" = "moddn" }
            { "newrdn" = "cn=bar" }
            { "deleteoldrdn" = "0" }
            { "newsuperior" = "dc=example,dc=net" } }
    }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
