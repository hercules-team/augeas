(*
Module: Test_sssd
  Test cases for the sssd lense

Author: Erinn Looney-Triggs

About: License
  This file is licensed under the LGPLv2+, like the rest of Augeas.
*)
module Test_sssd =

let conf = "[domain/example.com]
#Comment here
; another comment
cache_credentials = True
krb5_store_password_if_offline = True
ipa_server = _srv_, ipa.example.com
[sssd]
services = nss, pam
config_file_version = 2

domains = example.com
[nss]

[pam]
"

test Sssd.lns get conf =
    { "target" = "domain/example.com" 
	{ "#comment" = "Comment here" }
	{ "#comment" = "another comment" }
	{ "cache_credentials" = "True" }
	{ "krb5_store_password_if_offline" = "True" }
	{ "ipa_server" = "_srv_, ipa.example.com" }
    }
    { "target" = "sssd"
	{ "services" = "nss, pam" }
	{ "config_file_version" = "2" }
	{	}
	{ "domains" = "example.com" }
    }
    { "target" = "nss" 
	{	}
    }
    { "target" = "pam" }
