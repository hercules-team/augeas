module Test_Opendkim =

  let simple_string_value = "ADSPAction discard\n"
  test Opendkim.lns get simple_string_value =
    { "ADSPAction" = "discard" } 
  test Opendkim.lns put simple_string_value after
    set "ADSPAction" "discard" = simple_string_value 

  let simple_integer_value = "AutoRestartCount 1\n"
  test Opendkim.lns get simple_integer_value =
    { "AutoRestartCount" = "1" } 
  test Opendkim.lns put simple_integer_value after
    set "AutoRestartCount" "1" = simple_integer_value

  let simple_boolean_value = "AddAllSignatureResults true\n"
  test Opendkim.lns get simple_boolean_value =
    { "AddAllSignatureResults" = "true" } 
  test Opendkim.lns put simple_boolean_value after
    set "AddAllSignatureResults" "true" = simple_boolean_value

  let yes_boolean_value= "AddAllSignatureResults yes\n"
  test Opendkim.lns get yes_boolean_value =
    { "AddAllSignatureResults" = "yes" } 
  test Opendkim.lns put yes_boolean_value after
    set "AddAllSignatureResults" "yes" = yes_boolean_value

  let one_boolean_value= "AddAllSignatureResults 1\n"
  test Opendkim.lns get one_boolean_value =
    { "AddAllSignatureResults" = "1" } 
  test Opendkim.lns put one_boolean_value after
    set "AddAllSignatureResults" "1" = one_boolean_value

  let three_type_value = "AddAllSignatureResults false\nADSPAction discard\nAutoRestartCount 2\n"
  test Opendkim.lns get three_type_value =
    { "AddAllSignatureResults" = "false" }
    { "ADSPAction" = "discard" }
    { "AutoRestartCount" = "2" } 

  test Opendkim.lns put "" after
    set "AddAllSignatureResults" "false";
    set "ADSPAction" "discard";
    set "AutoRestartCount" "2" = three_type_value

  let two_boolean_value = "AddAllSignatureResults false\nADSPNoSuchDomain true\n"
  test Opendkim.lns get two_boolean_value =
    { "AddAllSignatureResults" = "false" }
    { "ADSPNoSuchDomain" = "true" }

  test Opendkim.lns put "" after
    set "AddAllSignatureResults" "false";
    set "ADSPNoSuchDomain" "true" = two_boolean_value 

  let blank_line_between= "AddAllSignatureResults false\n\nADSPNoSuchDomain true\n"
  test Opendkim.lns get blank_line_between =
    { "AddAllSignatureResults" = "false" }
    { }
    { "ADSPNoSuchDomain" = "true" }

  test Opendkim.lns put "" after
    set "AddAllSignatureResults" "false";
    set "ADSPNoSuchDomain" "true" = "AddAllSignatureResults false\nADSPNoSuchDomain true\n" 

  let include_comment_line= "AddAllSignatureResults false\n#A comment\nADSPNoSuchDomain true\n"
  test Opendkim.lns get include_comment_line =
    { "AddAllSignatureResults" = "false" }
    { "#comment" = "A comment" }
    { "ADSPNoSuchDomain" = "true" }

  test Opendkim.lns put "" after
    set "AddAllSignatureResults" "false";
    set "#comment" "A comment";
    set "ADSPNoSuchDomain" "true" = include_comment_line  

  let default_config_file = "# This is a basic configuration that can easily be adapted to suit a standard
# installation. For more advanced options, see opendkim.conf(5) and/or
# /usr/share/doc/opendkim/examples/opendkim.conf.sample.

# Log to syslog
Syslog      yes
# Required to use local socket with MTAs that access the socket as a non-
# privileged user (e.g. Postfix)
UMask     002

# Sign for example.com with key in /etc/mail/dkim.key using
# selector '2007' (e.g. 2007._domainkey.example.com)
#Domain     example.com
#KeyFile    /etc/mail/dkim.key
#Selector   2007

# Commonly-used options; the commented-out versions show the defaults.
#Canonicalization simple
#Mode     sv
#SubDomains   no
#ADSPAction            continue

# Always oversign From (sign using actual From and a null From to prevent
# malicious signatures header fields (From and/or others) between the signer
# and the verifier.  From is oversigned by default in the Debian package
# because it is often the identity key used by reputation systems and thus
# somewhat security sensitive.
OversignHeaders   From

# List domains to use for RFC 6541 DKIM Authorized Third-Party Signatures
# (ATPS) (experimental)

#ATPSDomains    example.com
"
  test Opendkim.lns get default_config_file = 
  { "#comment" = "This is a basic configuration that can easily be adapted to suit a standard" }
  { "#comment" = "installation. For more advanced options, see opendkim.conf(5) and/or" }
  { "#comment" = "/usr/share/doc/opendkim/examples/opendkim.conf.sample." }
  {  }
  { "#comment" = "Log to syslog" }
  { "Syslog" = "yes" }
  { "#comment" = "Required to use local socket with MTAs that access the socket as a non-" }
  { "#comment" = "privileged user (e.g. Postfix)" }
  { "UMask" = "002" }
  {  }
  { "#comment" = "Sign for example.com with key in /etc/mail/dkim.key using" }
  { "#comment" = "selector '2007' (e.g. 2007._domainkey.example.com)" }
  { "#comment" = "Domain     example.com" }
  { "#comment" = "KeyFile    /etc/mail/dkim.key" }
  { "#comment" = "Selector   2007" }
  {  }
  { "#comment" = "Commonly-used options; the commented-out versions show the defaults." }
  { "#comment" = "Canonicalization simple" }
  { "#comment" = "Mode     sv" }
  { "#comment" = "SubDomains   no" }
  { "#comment" = "ADSPAction            continue" }
  {  }
  { "#comment" = "Always oversign From (sign using actual From and a null From to prevent" }
  { "#comment" = "malicious signatures header fields (From and/or others) between the signer" }
  { "#comment" = "and the verifier.  From is oversigned by default in the Debian package" }
  { "#comment" = "because it is often the identity key used by reputation systems and thus" }
  { "#comment" = "somewhat security sensitive." }
  { "OversignHeaders" = "From" }
  {  }
  { "#comment" = "List domains to use for RFC 6541 DKIM Authorized Third-Party Signatures" }
  { "#comment" = "(ATPS) (experimental)" }
  {  }
  { "#comment" = "ATPSDomains    example.com" }
