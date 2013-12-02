module Test_cyrus_imapd =

let conf = "configdirectory: /var/lib/imap
partition-default: /var/spool/imap
admins: cyrus-admin
sievedir: /var/lib/imap/sieve
sendmail: /usr/sbin/sendmail
sasl_pwcheck_method: auxprop saslauthd
sasl_mech_list: PLAIN LOGIN
allowplaintext: no
tls_cert_file: /etc/pki/cyrus-imapd/cyrus-imapd.pem
tls_key_file: /etc/pki/cyrus-imapd/cyrus-imapd.pem
tls_ca_file: /etc/pki/tls/certs/ca-bundle.crt
# uncomment this if you're operating in a DSCP environment (RFC-4594)
# qosmarking: af13\n"

test Cyrus_Imapd.lns get conf =
   { "configdirectory"  = "/var/lib/imap" }
   { "partition-default"  = "/var/spool/imap" }
   { "admins" = "cyrus-admin" }
   { "sievedir" = "/var/lib/imap/sieve" }
   { "sendmail" = "/usr/sbin/sendmail"  }
   { "sasl_pwcheck_method" = "auxprop saslauthd" }
   { "sasl_mech_list" = "PLAIN LOGIN" }
   { "allowplaintext" = "no" }
   { "tls_cert_file" = "/etc/pki/cyrus-imapd/cyrus-imapd.pem" }
   { "tls_key_file" = "/etc/pki/cyrus-imapd/cyrus-imapd.pem" }
   { "tls_ca_file" = "/etc/pki/tls/certs/ca-bundle.crt" }
   { "#comment" = "uncomment this if you're operating in a DSCP environment (RFC-4594)" }
   { "#comment" = "qosmarking: af13" }

test Cyrus_Imapd.lns get "admins: cyrus-admin\n"
     =
  { "admins" = "cyrus-admin" }

test Cyrus_Imapd.lns put "" after set "munge8bit" "false" =
  "munge8bit: false\n"
