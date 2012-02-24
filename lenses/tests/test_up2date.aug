(*
Module: Test_Up2date
  Provides unit tests and examples for the <Up2date> lens.
*)

module Test_Up2date =

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
    { "1" = "Whether or not debugging is enabled" } }
  { "2" = "debug"
    { "1" = "0" } }
  {  }
  { "3" = "systemIdPath[comment]"
    { "1" = "Location of system id" } }
  { "4" = "systemIdPath"
    { "1" = "/etc/sysconfig/rhn/systemid" } }
  {  }
  { "5" = "serverURL[comment]"
    { "1" = "Remote server URL (use FQDN)" } }
  { "#comment" = "serverURL=https://xmlrpc.rhn.redhat.com/XMLRPC" }
  { "6" = "serverURL"
    { "1" = "https://enter.your.server.url.here/XMLRPC" } }
  {  }
  { "7" = "hostedWhitelist[comment]"
    { "1" = "RHN Hosted URL's" } }
  { "8" = "hostedWhitelist" }
  {  }
  { "9" = "enableProxy[comment]"
    { "1" = "Use a HTTP Proxy" } }
  { "10" = "enableProxy"
    { "1" = "0" } }
  {  }
  { "11" = "versionOverride[comment]"
    { "1" = "Override the automatically determined system version" } }
  { "12" = "versionOverride" }
  {  }
  { "13" = "httpProxy[comment]"
    { "1" = "HTTP proxy in host:port format, e.g. squid.redhat.com:3128" } }
  { "14" = "httpProxy" }
  {  }
  { "15" = "noReboot[comment]"
    { "1" = "Disable the reboot actions" } }
  { "16" = "noReboot"
    { "1" = "0" } }
  {  }
  { "17" = "networkRetries[comment]"
    { "1" = "Number of attempts to make at network connections before giving up" } }
  { "18" = "networkRetries"
    { "1" = "1" } }
  {  }
  { "19" = "disallowConfChanges[comment]"
    { "1" = "Config options that can not be overwritten by a config update action" } }
  { "20" = "disallowConfChanges"
    { "1" = "noReboot" }
    { "2" = "sslCACert" }
    { "3" = "useNoSSLForPackages" }
    { "4" = "noSSLServerURL" }
    { "5" = "serverURL" }
    { "6" = "disallowConfChanges" } }
  {  }
  { "21" = "sslCACert[comment]"
    { "1" = "The CA cert used to verify the ssl server" } }
  { "22" = "sslCACert"
    { "1" = "/usr/share/rhn/RHNS-CA-CERT" } }
  {  }
  { "#comment" = "Akamai does not support http protocol, therefore setting this option as side effect disable \"Location aware\" function" }
  { "23" = "useNoSSLForPackages[comment]"
    { "1" = "Use the noSSLServerURL for package, package list, and header fetching (disable Akamai)" } }
  { "24" = "useNoSSLForPackages"
    { "1" = "0" } }
  {  }
  { "25" = "retrieveOnly[comment]"
    { "1" = "Retrieve packages only" } }
  { "26" = "retrieveOnly"
    { "1" = "0" } }
  {  }
  { "27" = "skipNetwork[comment]"
    { "1" = "Skips network information in hardware profile sync during registration." } }
  { "28" = "skipNetwork"
    { "1" = "0" } }
  {  }
  { "29" = "tmpDir[comment]"
    { "1" = "Use this Directory to place the temporary transport files" } }
  { "30" = "tmpDir"
    { "1" = "/tmp" } }
  {  }
  { "31" = "writeChangesToLog[comment]"
    { "1" = "Log to /var/log/up2date which packages has been added and removed" } }
  { "32" = "writeChangesToLog"
    { "1" = "0" } }
  {  }
  { "33" = "stagingContent[comment]"
    { "1" = "Retrieve content of future actions in advance" } }
  { "34" = "stagingContent"
    { "1" = "1" } }
  {  }
  { "35" = "stagingContentWindow[comment]"
    { "1" = "How much forward we should look for future actions. In hours." } }
  { "36" = "stagingContentWindow"
    { "1" = "24" } }

