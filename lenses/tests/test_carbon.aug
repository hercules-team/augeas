(*
Module: Test_Carbon
  Provides unit tests and examples for the <Carbon> lens.
*)

module Test_Carbon =

let carbon_conf = "[cache]
# Configure carbon directories.

# Specify the user to drop privileges to
# If this is blank carbon runs as the user that invokes it
# This user must have write access to the local data directory
USER =

MAX_CACHE_SIZE = inf # comment at EOL
LINE_RECEIVER_INTERFACE=0.0.0.0
LINE_RECEIVER_PORT = 2003
ENABLE_UDP_LISTENER = False

[relay]
LINE_RECEIVER_INTERFACE = 0.0.0.0
LINE_RECEIVER_PORT = 2013
PICKLE_RECEIVER_INTERFACE = 0.0.0.0
PICKLE_RECEIVER_PORT = 2014
"

test Carbon.lns get carbon_conf =
  { "cache"
    { "#comment" = "Configure carbon directories." }
    { }
    { "#comment" = "Specify the user to drop privileges to" }
    { "#comment" = "If this is blank carbon runs as the user that invokes it" }
    { "#comment" = "This user must have write access to the local data directory" }
    { "USER" }
    { }
    { "MAX_CACHE_SIZE" = "inf"
      { "#comment" = "comment at EOL" }
    }
    { "LINE_RECEIVER_INTERFACE" = "0.0.0.0" }
    { "LINE_RECEIVER_PORT" = "2003" }
    { "ENABLE_UDP_LISTENER" = "False" }
    { }
  }
  { "relay"
    { "LINE_RECEIVER_INTERFACE" = "0.0.0.0" }
    { "LINE_RECEIVER_PORT" = "2013" }
    { "PICKLE_RECEIVER_INTERFACE" = "0.0.0.0" }
    { "PICKLE_RECEIVER_PORT" = "2014" }
  }

let relay_rules_conf = "# You must have exactly one section with 'default = true'
# Note that all destinations listed must also exist in carbon.conf
# in the DESTINATIONS setting in the [relay] section
[default]
default = true
destinations = 127.0.0.1:2004:a, 127.0.0.1:2104:b
"

test Carbon.lns get relay_rules_conf =
  { "#comment" = "You must have exactly one section with 'default = true'" }
  { "#comment" = "Note that all destinations listed must also exist in carbon.conf" }
  { "#comment" = "in the DESTINATIONS setting in the [relay] section" }
  { "default"
    { "default" = "true" }
    { "destinations" = "127.0.0.1:2004:a, 127.0.0.1:2104:b" }
  }

let storage_aggregation_conf = "# Aggregation methods for whisper files. Entries are scanned in order,
# and first match wins. This file is scanned for changes every 60 seconds
[max]
pattern = \.max$
xFilesFactor = 0.1
aggregationMethod = max
"

test Carbon.lns get storage_aggregation_conf =
  { "#comment" = "Aggregation methods for whisper files. Entries are scanned in order," }
  { "#comment" = "and first match wins. This file is scanned for changes every 60 seconds" }
  { "max"
    { "pattern" = "\.max$" }
    { "xFilesFactor" = "0.1" }
    { "aggregationMethod" = "max" }
  }
