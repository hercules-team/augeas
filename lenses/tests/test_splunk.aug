(**
 *
 * This module is used to test the Splunk module for valid extractions.
 * Written by Tim Brigham.
 * This file is licensed under the LGPLv2+, like the rest of Augeas.
 **)

module Test_splunk =

(** inputs.conf **)

   let inputs = "[default]
host = splunk-node-1.example.com
enable_autocomplete_login = False

[udp://514]
connection_host = none
source = test
sourcetype = syslog

"
test Splunk.lns get inputs =
  { "target" = "default"
      { "host" = "splunk-node-1.example.com" }
      { "enable_autocomplete_login" = "False" }
  {}}
  { "target" = "udp://514"
      { "connection_host" = "none" }
      { "source" = "test" }
      { "sourcetype" = "syslog" }
  {}}


(** web.conf **)
   let web = "[settings]
enableSplunkWebSSL = 1
enable_autocomplete_login = False
"


test Splunk.lns get web =
  { "target" = "settings"
      { "enableSplunkWebSSL" = "1" }
      { "enable_autocomplete_login" = "False" }
      }



(** props.conf **)

   let props = "[splunkd_stdout]
PREFIX_SOURCETYPE = False
SHOULD_LINEMERGE = False
is_valid = False
maxDist = 99

"

test Splunk.lns get props =
  {
    "target" = "splunkd_stdout"
             { "PREFIX_SOURCETYPE" = "False" }
             { "SHOULD_LINEMERGE" = "False" }
             { "is_valid" = "False" }
             { "maxDist" = "99" }
             {}}

(** tenants.conf **)
   let tenants = "[tenant:default]
whitelist.0 = *
"

test Splunk.lns get tenants =
  { "target" = "tenant:default"
     { "whitelist.0" = "*" }
  }



  let server = "[license]
active_group = Free
master_uri = https://myserver.mydomain.com:8089

[general]
serverName = splunk-node-1
trustedIP = 127.0.0.1
guid = XXXXXXX-XXXXXXX-XXXXXXX-XXXXXXX-XXXXXXXXXXXXX

[sslConfig]
sslKeysfilePassword = $1$XX2X4XX6XXXXXXXXX

"

test Splunk.lns get server =
    { "target" = "license"
        { "active_group" = "Free" }
        { "master_uri" = "https://myserver.mydomain.com:8089" }
        {}}
    { "target" = "general"
      { "serverName" = "splunk-node-1" }
      { "trustedIP" = "127.0.0.1" }
      { "guid" = "XXXXXXX-XXXXXXX-XXXXXXX-XXXXXXX-XXXXXXXXXXXXX" }
      {}}
    { "target" = "sslConfig"
      { "sslKeysfilePassword" = "$1$XX2X4XX6XXXXXXXXX" }
      {}}


(* test anonymous attributes *)
let anon = "
# master
serverName = splunk-node-1

# slave
serverName = splunk-node-2

[general]
serverName = splunk-node-3

"

test Splunk.lns get anon =
  { ".anon"
    {  }
    { "#comment" = "master" }
    { "serverName" = "splunk-node-1" }
    {  }
    { "#comment" = "slave" }
    { "serverName" = "splunk-node-2" }
    {  }
  }
  { "target" = "general"
    { "serverName" = "splunk-node-3" }
    {  }
  }


(* test empty value entry *)

let override = "
[general]
# normal entry
foo = bar
# override entry
foo =
"

test Splunk.lns get override =
  { ".anon"
    {  }
  }
  { "target" = "general"
    { "#comment" = "normal entry" }
    { "foo" = "bar" }
    { "#comment" = "override entry" }
    { "foo" }
  }

