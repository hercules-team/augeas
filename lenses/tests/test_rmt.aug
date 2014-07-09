module Test_rmt =

let conf = "#ident @(#)rmt.dfl	1.2 05/08/09 Copyr 2000 J. Schilling
#
# This file is /etc/default/rmt

DEBUG=/tmp/RMT
USER=*

ACCESS=rtape	sparky	/dev/rmt/*
ACCESS=*	*	/dev/rmt/*

# Historically, Red Hat rmt was not so ^^ restrictive.
ACCESS=*	*	*
"

test Rmt.lns get conf =
  { "#comment" = "ident @(#)rmt.dfl	1.2 05/08/09 Copyr 2000 J. Schilling" }
  { }
  { "#comment" = "This file is /etc/default/rmt" }
  { }
  { "DEBUG" = "/tmp/RMT" }
  { "USER" = "*" }
  { }
  { "ACCESS"
    { "name" = "rtape" }
    { "host" = "sparky" }
    { "path" = "/dev/rmt/*" } }
  { "ACCESS"
    { "name" = "*" }
    { "host" = "*" }
    { "path" = "/dev/rmt/*" } }
  { }
  { "#comment" = "Historically, Red Hat rmt was not so ^^ restrictive." }
  { "ACCESS"
    { "name" = "*" }
    { "host" = "*" }
    { "path" = "*" } }
