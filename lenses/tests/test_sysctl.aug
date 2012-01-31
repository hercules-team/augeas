(*
Module: Test_Sysctl
  Provides unit tests and examples for the <Sysctl> lens.
*)

module Test_sysctl =

(* Variable: default_sysctl *)
let default_sysctl = "# Kernel sysctl configuration file
# Controls IP packet forwarding
net.ipv4.ip_forward = 0

net.ipv4.conf.default.rp_filter = 1
net.ipv4.conf.default.accept_source_route = \t0
kernel.sysrq  =  0

; Semicolon comments are also allowed
net.ipv4.tcp_mem = \t393216 524288 786432
"

(* Test: Sysctl.lns *)
test Sysctl.lns get default_sysctl =
    { "#comment" = "Kernel sysctl configuration file" }
    { "#comment" = "Controls IP packet forwarding"}
    { "net.ipv4.ip_forward" = "0" }
    { }
    { "net.ipv4.conf.default.rp_filter" = "1" }
    { "net.ipv4.conf.default.accept_source_route" = "0" }
    { "kernel.sysrq" = "0" }
    { }
    { "#comment" = "Semicolon comments are also allowed" }
    { "net.ipv4.tcp_mem" = "393216 524288 786432" }

(* Test: Sysctl.lns *)
test Sysctl.lns put default_sysctl after
    set "net.ipv4.ip_forward" "1" ;
    rm "net.ipv4.conf.default.rp_filter" ;
    rm "net.ipv4.conf.default.accept_source_route" ;
    rm "kernel.sysrq"
  = "# Kernel sysctl configuration file
# Controls IP packet forwarding
net.ipv4.ip_forward = 1


; Semicolon comments are also allowed
net.ipv4.tcp_mem = \t393216 524288 786432
"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
