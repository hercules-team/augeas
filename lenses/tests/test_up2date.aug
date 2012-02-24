(*
Module: Test_Up2date
  Provides unit tests and examples for the <Up2date> lens.
*)

module Test_Up2date =

(* Variable: empty *)
let empty = "keyword=
"
test Up2date.lns get empty =
  { "1" = "keyword" }

(* Variable: list_empty *)
let list_empty = "keyword=;
"
test Up2date.lns get list_empty =
  { "1" = "keyword"
    { "values" } }

(* Variable: list_one *)
let list_one = "keyword=foo;
"
test Up2date.lns get list_one =
  { "1" = "keyword"
    { "values"
      { "1" = "foo" } } }

(* Variable: list_two
   Probably not useful, up2date throws "bar" away *)
let list_two = "keyword=foo;bar
"
test Up2date.lns get list_two =
  { "1" = "keyword"
    { "values"
      { "1" = "foo" }
      { "2" = "bar" } } }

(* Variable: list_two_trailing *)
let list_two_trailing = "keyword=foo;bar;
"
test Up2date.lns get list_two_trailing =
  { "1" = "keyword"
    { "values"
      { "1" = "foo" }
      { "2" = "bar" } } }

(* Variable: conf *)
let conf = "# Red Hat Update Agent config file.
# Format: 1.0

debug[comment]=Whether or not debugging is enabled
debug=0

systemIdPath[comment]=Location of system id
systemIdPath=/etc/sysconfig/rhn/systemid

serverURL[comment]=Remote server URL (use FQDN)
#serverURL=https://xmlrpc.rhn.redhat.com/XMLRPC
serverURL=https://enter.your.server.url.here/XMLRPC

hostedWhitelist[comment]=RHN Hosted URL's
hostedWhitelist=

enableProxy[comment]=Use a HTTP Proxy
enableProxy=0

versionOverride[comment]=Override the automatically determined system version
versionOverride=

httpProxy[comment]=HTTP proxy in host:port format, e.g. squid.redhat.com:3128
httpProxy=

noReboot[comment]=Disable the reboot actions
noReboot=0

networkRetries[comment]=Number of attempts to make at network connections before giving up
networkRetries=1

disallowConfChanges[comment]=Config options that can not be overwritten by a config update action
disallowConfChanges=noReboot;sslCACert;useNoSSLForPackages;noSSLServerURL;serverURL;disallowConfChanges;

sslCACert[comment]=The CA cert used to verify the ssl server
sslCACert=/usr/share/rhn/RHNS-CA-CERT

# Akamai does not support http protocol, therefore setting this option as side effect disable \"Location aware\" function
useNoSSLForPackages[comment]=Use the noSSLServerURL for package, package list, and header fetching (disable Akamai)
useNoSSLForPackages=0

retrieveOnly[comment]=Retrieve packages only
retrieveOnly=0

skipNetwork[comment]=Skips network information in hardware profile sync during registration.
skipNetwork=0

tmpDir[comment]=Use this Directory to place the temporary transport files
tmpDir=/tmp

writeChangesToLog[comment]=Log to /var/log/up2date which packages has been added and removed
writeChangesToLog=0

stagingContent[comment]=Retrieve content of future actions in advance
stagingContent=1

stagingContentWindow[comment]=How much forward we should look for future actions. In hours.
stagingContentWindow=24
"

(* Test: Up2date.lns *)
test Up2date.lns get conf =
  { "#comment" = "Red Hat Update Agent config file." }
  { "#comment" = "Format: 1.0" }
  {  }
  { "1" = "debug[comment]"
    { "value" = "Whether or not debugging is enabled" } }
  { "2" = "debug"
    { "value" = "0" } }
  {  }
  { "3" = "systemIdPath[comment]"
    { "value" = "Location of system id" } }
  { "4" = "systemIdPath"
    { "value" = "/etc/sysconfig/rhn/systemid" } }
  {  }
  { "5" = "serverURL[comment]"
    { "value" = "Remote server URL (use FQDN)" } }
  { "#comment" = "serverURL=https://xmlrpc.rhn.redhat.com/XMLRPC" }
  { "6" = "serverURL"
    { "value" = "https://enter.your.server.url.here/XMLRPC" } }
  {  }
  { "7" = "hostedWhitelist[comment]"
    { "value" = "RHN Hosted URL's" } }
  { "8" = "hostedWhitelist" }
  {  }
  { "9" = "enableProxy[comment]"
    { "value" = "Use a HTTP Proxy" } }
  { "10" = "enableProxy"
    { "value" = "0" } }
  {  }
  { "11" = "versionOverride[comment]"
    { "value" = "Override the automatically determined system version" } }
  { "12" = "versionOverride" }
  {  }
  { "13" = "httpProxy[comment]"
    { "value" = "HTTP proxy in host:port format, e.g. squid.redhat.com:3128" } }
  { "14" = "httpProxy" }
  {  }
  { "15" = "noReboot[comment]"
    { "value" = "Disable the reboot actions" } }
  { "16" = "noReboot"
    { "value" = "0" } }
  {  }
  { "17" = "networkRetries[comment]"
    { "value" = "Number of attempts to make at network connections before giving up" } }
  { "18" = "networkRetries"
    { "value" = "1" } }
  {  }
  { "19" = "disallowConfChanges[comment]"
    { "value" = "Config options that can not be overwritten by a config update action" } }
  { "20" = "disallowConfChanges"
    { "values"
      { "1" = "noReboot" }
      { "2" = "sslCACert" }
      { "3" = "useNoSSLForPackages" }
      { "4" = "noSSLServerURL" }
      { "5" = "serverURL" }
      { "6" = "disallowConfChanges" } } }
  {  }
  { "21" = "sslCACert[comment]"
    { "value" = "The CA cert used to verify the ssl server" } }
  { "22" = "sslCACert"
    { "value" = "/usr/share/rhn/RHNS-CA-CERT" } }
  {  }
  { "#comment" = "Akamai does not support http protocol, therefore setting this option as side effect disable \"Location aware\" function" }
  { "23" = "useNoSSLForPackages[comment]"
    { "value" = "Use the noSSLServerURL for package, package list, and header fetching (disable Akamai)" } }
  { "24" = "useNoSSLForPackages"
    { "value" = "0" } }
  {  }
  { "25" = "retrieveOnly[comment]"
    { "value" = "Retrieve packages only" } }
  { "26" = "retrieveOnly"
    { "value" = "0" } }
  {  }
  { "27" = "skipNetwork[comment]"
    { "value" = "Skips network information in hardware profile sync during registration." } }
  { "28" = "skipNetwork"
    { "value" = "0" } }
  {  }
  { "29" = "tmpDir[comment]"
    { "value" = "Use this Directory to place the temporary transport files" } }
  { "30" = "tmpDir"
    { "value" = "/tmp" } }
  {  }
  { "31" = "writeChangesToLog[comment]"
    { "value" = "Log to /var/log/up2date which packages has been added and removed" } }
  { "32" = "writeChangesToLog"
    { "value" = "0" } }
  {  }
  { "33" = "stagingContent[comment]"
    { "value" = "Retrieve content of future actions in advance" } }
  { "34" = "stagingContent"
    { "value" = "1" } }
  {  }
  { "35" = "stagingContentWindow[comment]"
    { "value" = "How much forward we should look for future actions. In hours." } }
  { "36" = "stagingContentWindow"
    { "value" = "24" } }


