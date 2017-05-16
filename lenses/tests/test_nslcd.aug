(*
Module: Test_Nslcd
  Provides unit tests and examples for the <Nslcd> lens.
*)

module Test_nslcd =

let real_file = "# /etc/nslcd.conf
# nslcd configuration file. See nslcd.conf(5)
# for details.

# Specifies the number of threads to start that can handle requests and perform LDAP queries.
threads 5

# The user and group nslcd should run as.
uid nslcd
gid nslcd

# This option controls the way logging is done.
log syslog info

# The location at which the LDAP server(s) should be reachable.
uri ldaps://XXX.XXX.XXX ldaps://YYY.YYY.YYY

# The search base that will be used for all queries.
base dc=XXX,dc=XXX

# The LDAP protocol version to use.
ldap_version 3

# The DN to bind with for normal lookups.
binddn cn=annonymous,dc=example,dc=net
bindpw secret


# The DN used for password modifications by root.
rootpwmoddn cn=admin,dc=example,dc=com

# The password used for password modifications by root.
rootpwmodpw XXXXXX


# SASL authentication options
sasl_mech OTP
sasl_realm realm
sasl_authcid authcid
sasl_authzid dn:cn=annonymous,dc=example,dc=net
sasl_secprops noanonymous,noplain,minssf=0,maxssf=2,maxbufsize=65535
sasl_canonicalize yes

# Kerberos authentication options
krb5_ccname ccname

# Search/mapping options

# Specifies the base distinguished name (DN) to use as search base.
base dc=people,dc=example,dc=com
base dc=morepeople,dc=example,dc=com
base alias dc=aliases,dc=example,dc=com
base alias dc=morealiases,dc=example,dc=com
base group dc=group,dc=example,dc=com
base group dc=moregroup,dc=example,dc=com
base passwd dc=users,dc=example,dc=com

# Specifies the search scope (subtree, onelevel, base or children).
scope sub
scope passwd sub
scope aliases sub

# Specifies the policy for dereferencing aliases.
deref never

# Specifies whether automatic referral chasing should be enabled.
referrals yes

# The FILTER is an LDAP search filter to use for a specific map.
filter group (objectClass=posixGroup)

# This option allows for custom attributes to be looked up instead of the default RFC 2307 attributes.
map passwd homeDirectory \"${homeDirectory:-/home/$uid}\"
map passwd loginShell \"${loginShell:-/bin/bash}\"
map shadow userPassword myPassword

# Timing/reconnect options

# Specifies the time limit (in seconds) to use when connecting to the directory server.
bind_timelimit 30

# Specifies the time limit (in seconds) to wait for a response from the LDAP server.
timelimit 5

# Specifies the period if inactivity (in seconds) after which the connection to the LDAP server will be closed.
idle_timelimit 10

# Specifies the number of seconds to sleep when connecting to all LDAP servers fails.
reconnect_sleeptime 10

# Specifies the time after which the LDAP server is considered to be permanently unavailable.
reconnect_retrytime 10

# SSL/TLS options

# Specifies whether to use SSL/TLS or not (the default is not to).
ssl start_tls
# Specifies what checks to perform on a server-supplied certificate.
tls_reqcert never
# Specifies the directory containing X.509 certificates for peer authentication.
tls_cacertdir /etc/ssl/ca
# Specifies the path to the X.509 certificate for peer authentication.
tls_cacertfile /etc/ssl/certs/ca-certificates.crt
# Specifies the path to an entropy source.
tls_randfile /dev/random
# Specifies the ciphers to use for TLS.
tls_ciphers TLSv1
# Specifies the path to the file containing the local certificate for client TLS authentication.
tls_cert /etc/ssl/certs/cert.pem
# Specifies the path to the file containing the private key for client TLS authentication.
tls_key /etc/ssl/private/cert.pem

# Other options
pagesize 100
nss_initgroups_ignoreusers user1,user2,user3
nss_min_uid 1000
nss_nested_groups yes
nss_getgrent_skipmembers yes
nss_disable_enumeration yes
validnames /^[a-z0-9._@$()]([a-z0-9._@$() \\~-]*[a-z0-9._@$()~-])?$/i
ignorecase yes
pam_authc_ppolicy yes
pam_authz_search (&(objectClass=posixAccount)(uid=$username)(|(authorizedService=$service)(!(authorizedService=*))))
pam_password_prohibit_message \"MESSAGE LONG AND WITH SPACES\"
reconnect_invalidate nfsidmap,db2,db3
cache dn2uid 1s 2h

"

test Nslcd.lns get real_file =
 { "#comment" = "/etc/nslcd.conf" }
  { "#comment" = "nslcd configuration file. See nslcd.conf(5)" }
  { "#comment" = "for details." }
  {  }
  { "#comment" = "Specifies the number of threads to start that can handle requests and perform LDAP queries." }
  { "threads" = "5" }
  {  }
  { "#comment" = "The user and group nslcd should run as." }
  { "uid" = "nslcd" }
  { "gid" = "nslcd" }
  {  }
  { "#comment" = "This option controls the way logging is done." }
  { "log" = "syslog info" }
  {  }
  { "#comment" = "The location at which the LDAP server(s) should be reachable." }
  { "uri"
    { "1" = "ldaps://XXX.XXX.XXX" }
    { "2" = "ldaps://YYY.YYY.YYY" }
  }
  {  }
  { "#comment" = "The search base that will be used for all queries." }
  { "base" = "dc=XXX,dc=XXX" }
  {  }
  { "#comment" = "The LDAP protocol version to use." }
  { "ldap_version" = "3" }
  {  }
  { "#comment" = "The DN to bind with for normal lookups." }
  { "binddn" = "cn=annonymous,dc=example,dc=net" }
  { "bindpw" = "secret" }
  {  }
  {  }
  { "#comment" = "The DN used for password modifications by root." }
  { "rootpwmoddn" = "cn=admin,dc=example,dc=com" }
  {  }
  { "#comment" = "The password used for password modifications by root." }
  { "rootpwmodpw" = "XXXXXX" }
  {  }
  {  }
  { "#comment" = "SASL authentication options" }
  { "sasl_mech" = "OTP" }
  { "sasl_realm" = "realm" }
  { "sasl_authcid" = "authcid" }
  { "sasl_authzid" = "dn:cn=annonymous,dc=example,dc=net" }
  { "sasl_secprops" = "noanonymous,noplain,minssf=0,maxssf=2,maxbufsize=65535" }
  { "sasl_canonicalize" = "yes" }
  {  }
  { "#comment" = "Kerberos authentication options" }
  { "krb5_ccname" = "ccname" }
  {  }
  { "#comment" = "Search/mapping options" }
  {  }
  { "#comment" = "Specifies the base distinguished name (DN) to use as search base." }
  { "base" = "dc=people,dc=example,dc=com" }
  { "base" = "dc=morepeople,dc=example,dc=com" }
  { "base"
    { "alias" = "dc=aliases,dc=example,dc=com" }
  }
  { "base"
    { "alias" = "dc=morealiases,dc=example,dc=com" }
  }
  { "base"
    { "group" = "dc=group,dc=example,dc=com" }
  }
  { "base"
    { "group" = "dc=moregroup,dc=example,dc=com" }
  }
  { "base"
    { "passwd" = "dc=users,dc=example,dc=com" }
  }
  {  }
  { "#comment" = "Specifies the search scope (subtree, onelevel, base or children)." }
  { "scope" = "sub" }
  { "scope"
    { "passwd" = "sub" }
  }
  { "scope"
    { "aliases" = "sub" }
  }
  {  }
  { "#comment" = "Specifies the policy for dereferencing aliases." }
  { "deref" = "never" }
  {  }
  { "#comment" = "Specifies whether automatic referral chasing should be enabled." }
  { "referrals" = "yes" }
  {  }
  { "#comment" = "The FILTER is an LDAP search filter to use for a specific map." }
  { "filter"
    { "group" = "(objectClass=posixGroup)" }
  }
  {  }
  { "#comment" = "This option allows for custom attributes to be looked up instead of the default RFC 2307 attributes." }
  { "map"
    { "passwd"
      { "homeDirectory" = "\"${homeDirectory:-/home/$uid}\"" }
    }
  }
  { "map"
    { "passwd"
      { "loginShell" = "\"${loginShell:-/bin/bash}\"" }
    }
  }
  { "map"
    { "shadow"
      { "userPassword" = "myPassword" }
    }
  }
  {  }
  { "#comment" = "Timing/reconnect options" }
  {  }
  { "#comment" = "Specifies the time limit (in seconds) to use when connecting to the directory server." }
  { "bind_timelimit" = "30" }
  {  }
  { "#comment" = "Specifies the time limit (in seconds) to wait for a response from the LDAP server." }
  { "timelimit" = "5" }
  {  }
  { "#comment" = "Specifies the period if inactivity (in seconds) after which the connection to the LDAP server will be closed." }
  { "idle_timelimit" = "10" }
  {  }
  { "#comment" = "Specifies the number of seconds to sleep when connecting to all LDAP servers fails." }
  { "reconnect_sleeptime" = "10" }
  {  }
  { "#comment" = "Specifies the time after which the LDAP server is considered to be permanently unavailable." }
  { "reconnect_retrytime" = "10" }
  {  }
  { "#comment" = "SSL/TLS options" }
  {  }
  { "#comment" = "Specifies whether to use SSL/TLS or not (the default is not to)." }
  { "ssl" = "start_tls" }
  { "#comment" = "Specifies what checks to perform on a server-supplied certificate." }
  { "tls_reqcert" = "never" }
  { "#comment" = "Specifies the directory containing X.509 certificates for peer authentication." }
  { "tls_cacertdir" = "/etc/ssl/ca" }
  { "#comment" = "Specifies the path to the X.509 certificate for peer authentication." }
  { "tls_cacertfile" = "/etc/ssl/certs/ca-certificates.crt" }
  { "#comment" = "Specifies the path to an entropy source." }
  { "tls_randfile" = "/dev/random" }
  { "#comment" = "Specifies the ciphers to use for TLS." }
  { "tls_ciphers" = "TLSv1" }
  { "#comment" = "Specifies the path to the file containing the local certificate for client TLS authentication." }
  { "tls_cert" = "/etc/ssl/certs/cert.pem" }
  { "#comment" = "Specifies the path to the file containing the private key for client TLS authentication." }
  { "tls_key" = "/etc/ssl/private/cert.pem" }
  {  }
  { "#comment" = "Other options" }
  { "pagesize" = "100" }
  { "nss_initgroups_ignoreusers"
    { "1" =  "user1" }
    { "2" =  "user2" }
    { "3" =  "user3" }
  }
  { "nss_min_uid" = "1000" }
  { "nss_nested_groups" = "yes" }
  { "nss_getgrent_skipmembers" = "yes" }
  { "nss_disable_enumeration" = "yes" }
  { "validnames" = "/^[a-z0-9._@$()]([a-z0-9._@$() \~-]*[a-z0-9._@$()~-])?$/i" }
  { "ignorecase" = "yes" }
  { "pam_authc_ppolicy" = "yes" }
  { "pam_authz_search" = "(&(objectClass=posixAccount)(uid=$username)(|(authorizedService=$service)(!(authorizedService=*))))" }
  { "pam_password_prohibit_message" = "MESSAGE LONG AND WITH SPACES" }
  { "reconnect_invalidate" = "nfsidmap,db2,db3" }
  { "cache" = "dn2uid 1s 2h" }
  {  }
(* Test writes *)

(* Test a simple parameter *)
test Nslcd.lns put "pagesize 9999\n" after
   set "/pagesize" "1000" =
   "pagesize 1000\n"

(* Test base parameter *)
test Nslcd.lns put "\n" after
   set "/base" "dc=example,dc=com" =
   "\nbase dc=example,dc=com\n"

test Nslcd.lns put "base dc=change,dc=me\n" after
   set "/base" "dc=example,dc=com" =
   "base dc=example,dc=com\n"

test Nslcd.lns put "\n" after
   set "/base/passwd" "dc=example,dc=com" =
   "\nbase passwd dc=example,dc=com\n"

test Nslcd.lns put "base passwd dc=change,dc=me\n" after
   set "/base[passwd]/passwd" "dc=example,dc=com";
   set "/base[shadow]/shadow" "dc=example,dc=com" =
   "base passwd dc=example,dc=com\nbase shadow dc=example,dc=com\n"

(* Test scope entry *)
test Nslcd.lns put "\n" after
   set "/scope" "sub" =
   "\nscope sub\n"

test Nslcd.lns put "scope one\n" after
   set "/scope" "subtree" =
   "scope subtree\n"

test Nslcd.lns put "\n" after
   set "/scope/passwd" "base" =
   "\nscope passwd base\n"

test Nslcd.lns put "scope shadow onelevel\n" after
   set "/scope[passwd]/passwd" "subtree";
   set "/scope[shadow]/shadow" "base" =
   "scope shadow base\nscope passwd subtree\n"

(* Test filter entry *)
test Nslcd.lns put "\n" after
   set "/filter/passwd" "(objectClass=posixAccount)" =
   "\nfilter passwd (objectClass=posixAccount)\n"

test Nslcd.lns put "filter shadow (objectClass=posixAccount)\n" after
   set "/filter[passwd]/passwd" "(objectClass=Account)";
   set "/filter[shadow]/shadow" "(objectClass=Account)" =
   "filter shadow (objectClass=Account)\nfilter passwd (objectClass=Account)\n"

(* Test map entry *)
test Nslcd.lns put "map passwd loginShell ab\n" after
   set "/map/passwd/loginShell" "bc" =
   "map passwd loginShell bc\n"

test Nslcd.lns put "map passwd loginShell ab\n" after
   set "/map[2]/passwd/homeDirectory" "bc" =
   "map passwd loginShell ab\nmap passwd homeDirectory bc\n"

test Nslcd.lns put "map passwd loginShell ab\n" after
   set "/map[passwd/homeDirectory]/passwd/homeDirectory" "bc" =
   "map passwd loginShell ab\nmap passwd homeDirectory bc\n"

test Nslcd.lns put "map passwd loginShell ab\nmap passwd homeDirectory ab\n" after
   set "/map[passwd/homeDirectory]/passwd/homeDirectory" "bc" =
   "map passwd loginShell ab\nmap passwd homeDirectory bc\n"


(* Test simple entries *)
let simple = "uid nslcd\n"

test Nslcd.lns get simple =
{ "uid" = "nslcd" }

(* Test simple entries with spaces at the end *)
let simple_spaces = "uid nslcd   \n"

test Nslcd.lns get simple_spaces =
{ "uid" = "nslcd" }

(* Test multi valued entries *)

let multi_valued = "cache 1 2    \n"

test Nslcd.lns get multi_valued =
{ "cache" = "1 2" }

let multi_valued_real = "map passwd homeDirectory ${homeDirectory:-/home/$uid}\n"

test Nslcd.lns get multi_valued_real =
{ "map"
  { "passwd"
    { "homeDirectory" = "${homeDirectory:-/home/$uid}" }
  }
}

(* Test multiline *)

let simple_multiline = "uid nslcd\ngid nslcd\n"

test Nslcd.lns get simple_multiline =
{"uid" = "nslcd"}
{"gid" = "nslcd"}


let multiline_separators  = "\n\n  \nuid nslcd    \ngid nslcd          \n"

test Nslcd.lns get multiline_separators =
{}
{}
{}
{"uid" = "nslcd"}
{"gid" = "nslcd"}
