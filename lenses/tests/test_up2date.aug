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
  { "debug"
    { "#comment" = "Whether or not debugging is enabled" }
  }
  { "debug" = "0" }
  {  }
  { "systemIdPath"
    { "#comment" = "Location of system id" }
  }
  { "systemIdPath" = "/etc/sysconfig/rhn/systemid" }
  {  }
  { "serverURL"
    { "#comment" = "Remote server URL (use FQDN)" }
  }
  { "#comment" = "serverURL=https://xmlrpc.rhn.redhat.com/XMLRPC" }
  { "serverURL" = "https://enter.your.server.url.here/XMLRPC" }
  {  }
  { "hostedWhitelist"
    { "#comment" = "RHN Hosted URL's" }
  }
  { "hostedWhitelist" }
  {  }
  { "enableProxy"
    { "#comment" = "Use a HTTP Proxy" }
  }
  { "enableProxy" = "0" }
  {  }
  { "versionOverride"
    { "#comment" = "Override the automatically determined system version" }
  }
  { "versionOverride" }
  {  }
  { "httpProxy"
    { "#comment" = "HTTP proxy in host:port format, e.g. squid.redhat.com:3128" }
  }
  { "httpProxy" }
  {  }
  { "noReboot"
    { "#comment" = "Disable the reboot actions" }
  }
  { "noReboot" = "0" }
  {  }
  { "networkRetries"
    { "#comment" = "Number of attempts to make at network connections before giving up" }
  }
  { "networkRetries" = "1" }
  {  }
  { "disallowConfChanges"
    { "#comment" = "Config options that can not be overwritten by a config update action" }
  }
  { "disallowConfChanges"
    { "1" = "noReboot" }
    { "2" = "sslCACert" }
    { "3" = "useNoSSLForPackages" }
    { "4" = "noSSLServerURL" }
    { "5" = "serverURL" }
    { "6" = "disallowConfChanges" }
  }
  {  }
  { "sslCACert"
    { "#comment" = "The CA cert used to verify the ssl server" }
  }
  { "sslCACert" = "/usr/share/rhn/RHNS-CA-CERT" }
  {  }
  { "#comment" = "Akamai does not support http protocol, therefore setting this option as side effect disable \"Location aware\" function" }
  { "useNoSSLForPackages"
    { "#comment" = "Use the noSSLServerURL for package, package list, and header fetching (disable Akamai)" }
  }
  { "useNoSSLForPackages" = "0" }
  {  }
  { "retrieveOnly"
    { "#comment" = "Retrieve packages only" }
  }
  { "retrieveOnly" = "0" }
  {  }
  { "skipNetwork"
    { "#comment" = "Skips network information in hardware profile sync during registration." }
  }
  { "skipNetwork" = "0" }
  {  }
  { "tmpDir"
    { "#comment" = "Use this Directory to place the temporary transport files" }
  }
  { "tmpDir" = "/tmp" }
  {  }
  { "writeChangesToLog"
    { "#comment" = "Log to /var/log/up2date which packages has been added and removed" }
  }
  { "writeChangesToLog" = "0" }
  {  }
  { "stagingContent"
    { "#comment" = "Retrieve content of future actions in advance" }
  }
  { "stagingContent" = "1" }
  {  }
  { "stagingContentWindow"
    { "#comment" = "How much forward we should look for future actions. In hours." }
  }
  { "stagingContentWindow" = "24" }

