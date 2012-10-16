(*
Module: Test_MCollective
  Provides unit tests and examples for the <MCollective> lens.
*)

module Test_MCollective =

let conf = "topicprefix = /topic/
main_collective = mcollective
collectives = mcollective
libdir = /usr/libexec/mcollective
logger_type = console
loglevel = warn

# Plugins
securityprovider = psk
plugin.psk = unset

connector = stomp
plugin.stomp.host = localhost
plugin.stomp.port = 61613
plugin.stomp.user = mcollective
plugin.stomp.password = secret

# Facts
factsource = yaml # bla
plugin.yaml=/etc/mcollective/facts.yaml
"

test MCollective.lns get conf =
  { "topicprefix" = "/topic/" }
  { "main_collective" = "mcollective" }
  { "collectives" = "mcollective" }
  { "libdir" = "/usr/libexec/mcollective" }
  { "logger_type" = "console" }
  { "loglevel" = "warn" }
  { }
  { "#comment" = "Plugins" }
  { "securityprovider" = "psk" }
  { "plugin.psk" = "unset" }
  { }
  { "connector" = "stomp" }
  { "plugin.stomp.host" = "localhost" }
  { "plugin.stomp.port" = "61613" }
  { "plugin.stomp.user" = "mcollective" }
  { "plugin.stomp.password" = "secret" }
  { }
  { "#comment" = "Facts" }
  { "factsource" = "yaml"
    { "#comment" = "bla" }
  }
  { "plugin.yaml" = "/etc/mcollective/facts.yaml" }

