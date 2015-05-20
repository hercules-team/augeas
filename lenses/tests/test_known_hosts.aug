(*
Module: Test_Known_Hosts
  Provides unit tests and examples for the <Known_Hosts> lens.
*)

module Test_Known_Hosts =

(* Test: Known_Hosts.lns
     Simple get test *)
test Known_Hosts.lns get "# A comment
foo.example.com,foo,bar ecdsa-sha2-nistp256 AAABBBDKDFX=
|1|FhUqf1kMlRWNfK6InQSAmXiNiSY=|jwbKFwD4ipl6D0k6OoshmW7xOao= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBIvNOU8OedkWalFmoFcJWP3nasnCLx6M78F9y0rzTQtplggNd0dvR0A4SQOBfHInmk5dH6YGGcpT3PM3cJBR7rI=\n" =
  { "#comment" = "A comment" }
  { "1" = "foo.example.com"
    { "alias" = "foo" }
    { "alias" = "bar" }
    { "type" = "ecdsa-sha2-nistp256" }
    { "key" = "AAABBBDKDFX=" } }
  { "2" = "|1|FhUqf1kMlRWNfK6InQSAmXiNiSY=|jwbKFwD4ipl6D0k6OoshmW7xOao="
    { "type" = "ecdsa-sha2-nistp256" }
    { "key" = "AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBIvNOU8OedkWalFmoFcJWP3nasnCLx6M78F9y0rzTQtplggNd0dvR0A4SQOBfHInmk5dH6YGGcpT3PM3cJBR7rI=" } }

