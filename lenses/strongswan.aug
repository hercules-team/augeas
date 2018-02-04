(*
Module: Strongswan
  Lens for parsing strongSwan configuration files

Authors:
  Kaarle Ritvanen <kaarle.ritvanen@datakunkku.fi>

About: Reference
  strongswan.conf(5)

About: License
  This file is licensed under the LGPL v2+
*)

module Strongswan =

autoload xfm

let ws = del /[\n\t ]*(#[\t ]*\n[\n\t ]*)*/

let rec conf =
	   let name (sep:string) =
	   	key (/[^\/.\{\}#\n\t ]+/ - /include/) . Util.del_ws_spc .
		Util.del_str sep
	in let val = store /[^\n\t ].*/ . Util.del_str "\n" . ws ""
	in let sval = Util.del_ws_spc . val
in (
	[ Util.del_str "#" . label "#comment" . Util.del_opt_ws " " . val ] |
	[ key "include" . sval ] |
	[ name "=" . sval ] |
	[ name "{" . ws "\n" . conf . Util.del_str "}" . ws "\n" ]
)*

let lns = ws "" . conf

let xfm = transform lns (
	incl "/etc/strongswan.d/*.conf" .
	incl "/etc/strongswan.d/**/*.conf" .
	incl "/etc/swanctl/conf.d/*.conf" .
	incl "/etc/swanctl/swanctl.conf"
)
