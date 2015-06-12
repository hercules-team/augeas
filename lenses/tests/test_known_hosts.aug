(*
Module: Test_Known_Hosts
  Provides unit tests and examples for the <Known_Hosts> lens.
*)

module Test_Known_Hosts =

(* Test: Known_Hosts.lns
     Simple get test *)
test Known_Hosts.lns get "# A comment
foo.example.com,foo ecdsa-sha2-nistp256 AAABBBDKDFX=
bar.example.com,bar ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIN9NJSjDZh4+K6WBS16iX7ZndnwbGsaEbLwHlCEhZmef
|1|FhUqf1kMlRWNfK6InQSAmXiNiSY=|jwbKFwD4ipl6D0k6OoshmW7xOao= ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBIvNOU8OedkWalFmoFcJWP3nasnCLx6M78F9y0rzTQtplggNd0dvR0A4SQOBfHInmk5dH6YGGcpT3PM3cJBR7rI=\n" =
  { "#comment" = "A comment" }
  { "1" = "foo.example.com"
    { "alias" = "foo" }
    { "type" = "ecdsa-sha2-nistp256" }
    { "key" = "AAABBBDKDFX=" } }
  { "2" = "bar.example.com"
    { "alias" = "bar" }
    { "type" = "ssh-ed25519" }
    { "key" = "AAAAC3NzaC1lZDI1NTE5AAAAIN9NJSjDZh4+K6WBS16iX7ZndnwbGsaEbLwHlCEhZmef" } }
  { "3" = "|1|FhUqf1kMlRWNfK6InQSAmXiNiSY=|jwbKFwD4ipl6D0k6OoshmW7xOao="
    { "type" = "ecdsa-sha2-nistp256" }
    { "key" = "AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAABBBIvNOU8OedkWalFmoFcJWP3nasnCLx6M78F9y0rzTQtplggNd0dvR0A4SQOBfHInmk5dH6YGGcpT3PM3cJBR7rI=" } }

(* Test: Known_Hosts.lns
      Markers *)
test Known_Hosts.lns get "@revoked * ssh-rsa AAAAB5W
@cert-authority *.mydomain.org,*.mydomain.com ssh-rsa AAAAB5W\n" =
  { "1" = "*"
    { "@revoked" }
    { "type" = "ssh-rsa" }
    { "key" = "AAAAB5W" } }
  { "2" = "*.mydomain.org"
    { "@cert-authority" }
    { "alias" = "*.mydomain.com" }
    { "type" = "ssh-rsa" }
    { "key" = "AAAAB5W" } }

(* Test: Known_Hosts.lns
      Eol comment *)
test Known_Hosts.lns get "@revoked * ssh-rsa AAAAB5W # this is revoked\n" =
  { "1" = "*"
    { "@revoked" }
    { "type" = "ssh-rsa" }
    { "key" = "AAAAB5W" }
    { "#comment" = "this is revoked" } }
