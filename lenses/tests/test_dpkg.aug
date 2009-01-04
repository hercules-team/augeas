module Test_dpkg =

let conf ="# dpkg configuration file
# Do not enable debsig-verify by default
no-debsig

log /var/log/dpkg.log\n"

test Dpkg.lns get conf =
  { "#comment" = "dpkg configuration file" }
  { "#comment" = "Do not enable debsig-verify by default" }
  { "no-debsig" }
  {}
  { "log" = "/var/log/dpkg.log" }
