module Test_star =

let conf = "# @(#)star.dfl  1.2 05/08/09 Copyright 2003 J. Schilling
#
# This file is /etc/default/star

STAR_FIFOSIZE=         32m

STAR_FIFOSIZE_MAX=     100m

archive0=/dev/rmt/0             20      0   N
archive1=/dev/rmt/0n            20      0   n
archive2=/dev/rmt/1             20      0   y
archive3=/dev/rmt/1n            20      0
archive4=/dev/rmt/0             126     0
archive5=/dev/rmt/0n            126     0
archive6=/dev/rmt/1             126     0
archive7=/dev/rmt/1n            126     0
"
test Star.lns get conf =
  { "#comment" = "@(#)star.dfl  1.2 05/08/09 Copyright 2003 J. Schilling" }
  { }
  { "#comment" = "This file is /etc/default/star" }
  { }
  { "STAR_FIFOSIZE" = "32m" }
  { }
  { "STAR_FIFOSIZE_MAX" = "100m" }
  { }
  { "archive0"
    { "device" = "/dev/rmt/0" }
    { "block" = "20" }
    { "size" = "0" }
    { "istape" = "N" } }
  { "archive1"
    { "device" = "/dev/rmt/0n" }
    { "block" = "20" }
    { "size" = "0" }
    { "istape" = "n" } }
  { "archive2"
    { "device" = "/dev/rmt/1" }
    { "block" = "20" }
    { "size" = "0" }
    { "istape" = "y" } }
  { "archive3"
    { "device" = "/dev/rmt/1n" }
    { "block" = "20" }
    { "size" = "0" } }
  { "archive4"
    { "device" = "/dev/rmt/0" }
    { "block" = "126" }
    { "size" = "0" } }
  { "archive5"
    { "device" = "/dev/rmt/0n" }
    { "block" = "126" }
    { "size" = "0" } }
  { "archive6"
    { "device" = "/dev/rmt/1" }
    { "block" = "126" }
    { "size" = "0" } }
  { "archive7"
    { "device" = "/dev/rmt/1n" }
    { "block" = "126" }
    { "size" = "0" } }
