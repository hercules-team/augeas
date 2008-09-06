module Test_ldap =

let conf = "host 127.0.0.1

# The distinguished name of the search base.
base dc=example,dc=com
#tls_key
ssl no
pam_password md5
"

test Spacevars.simple_lns get conf =
  { "host" = "127.0.0.1" }
  {}
  { "#comment" = "The distinguished name of the search base." }
  { "base" = "dc=example,dc=com" }
  { "#comment" = "tls_key" }
  { "ssl" = "no" }
  { "pam_password" = "md5" }

