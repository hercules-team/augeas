(* OpenVPN module for Augeas
 Author: Raphael Pinson <raphink@gmail.com>

 Reference: http://openvpn.net/index.php/documentation/howto.html
*)


module OpenVPN =
  autoload xfm

(************************************************************************
 *                           USEFUL PRIMITIVES
 *************************************************************************)

let eol    = Util.eol
let indent = Util.indent

(* Define separators *)
let sep    = Util.del_ws_spc

(* Define value regexps *)
let ip_re  = Rx.ipv4
let num_re = Rx.integer
let fn_re  = /[^#; \t\n][^#;\n]*[^#; \t\n]|[^#; \t\n]/
let an_re  = /[a-z][a-z0-9_-]*/

(* Define store aliases *)
let ip     = store ip_re
let num    = store num_re
let filename = store fn_re
let sto_to_dquote = store /[^"\n]+/   (* " Emacs, relax *)

(* define comments and empty lines *)
let comment = Util.comment_generic /[ \t]*[;#][ \t]*/ "# "
let comment_or_eol = eol | Util.comment_generic /[ \t]*[;#][ \t]*/ " # "

let empty   = Util.empty


(************************************************************************
 *                               SINGLE VALUES
 *
 *   - local => IP
 *   - port  => num
 *   - proto => tcp|udp
 *   - dev   => (tun|tap)\d*
 *   - dev-node => MyTap
 *   - ca    => filename
 *   - cert  => filename
 *   - key   => filename
 *   - dh    => filename
 *   - ifconfig-pool-persist => filename
 *   - learn-address => filename
 *   - cipher => [A-Z0-9-]+
 *   - max-clients => num
 *   - user  => alphanum
 *   - group => alphanum
 *   - status => filename
 *   - log   => filename
 *   - log-append => filename
 *   - client-config-dir => filename
 *   - verb => num
 *   - mute => num
 *   - fragment => num
 *   - mssfix   => num
 *   - ns-cert-type => "server"
 *   - resolv-retry => "infinite"
 *   - script-security => [0-3] (execve|system)?
 *************************************************************************)

let single_ip  = "local"
let single_num = "port"
               | "max-clients"
               | "verb"
	       | "mute"
               | "fragment"
               | "mssfix"
let single_fn  = "ca"
               | "cert"
	       | "key"
	       | "dh"
	       | "ifconfig-pool-persist"
	       | "learn-address"
	       | "status"
	       | "log"
	       | "log-append"
	       | "client-config-dir"
let single_an  = "user"
               | "group"


let single_entry (kw:regexp) (re:regexp)
               = [ key kw . sep . store re . comment_or_eol ]

let single     = single_entry single_num num_re
      	       | single_entry single_fn  fn_re
	       | single_entry single_an  an_re
	       | single_entry "local"    ip_re
	       | single_entry "proto"    /(tcp|udp)/
               | single_entry "dev"      /(tun|tap)[0-9]*/
	       | single_entry "dev-node" "MyTap"
	       | single_entry "cipher"   /[A-Z][A-Z0-9-]*/
	       | single_entry "ns-cert-type" "server"
	       | single_entry "resolv-retry" "infinite"
	       | single_entry "script-security" /[0-3]( execve| system)?/


(************************************************************************
 *                               FLAGS
 *
 *   - client-to-client
 *   - duplicate-cn
 *   - comp-lzo
 *   - persist-key
 *   - persist-tun
 *   - client
 *   - remote-random
 *   - nobind
 *   - mute-replay-warnings
 *   - http-proxy-retry
 *   - daemon
 *
 *************************************************************************)

let flag_words = "client-to-client"
               | "duplicate-cn"
	       | "comp-lzo"
	       | "persist-key"
	       | "persist-tun"
	       | "client"
	       | "remote-random"
	       | "nobind"
	       | "mute-replay-warnings"
	       | "http-proxy-retry"
	       | "daemon"

let flag_entry (kw:regexp)
               = [ key kw . comment_or_eol ]

let flag       = flag_entry flag_words


(************************************************************************
 *                               OTHER FIELDS
 *
 *   - server        => IP IP
 *   - server-bridge => IP IP IP IP
 *   - route	     => IP IP
 *   - push          => "string"
 *   - keepalive     => num num
 *   - tls-auth      => filename [01]
 *   - remote        => hostname/IP num
 *   - management    => IP num filename
 *
 *************************************************************************)

let server        = [ key "server" . sep
                    . [ label "address" . ip ] . sep
		    . [ label "netmask" . ip ] . comment_or_eol
		    ]

let server_bridge = [ key "server-bridge" . sep
                    . [ label "address" . ip ] . sep
		    . [ label "netmask" . ip ] . sep
		    . [ label "start"   . ip ] . sep
		    . [ label "end"     . ip ] . comment_or_eol
		    ]

let route         = [ key "route" . sep
                    . [ label "address" . ip ] . sep
                    . [ label "netmask" . ip ] . comment_or_eol
                    ]

let push          = [ key "push" . sep
                    . Quote.do_dquote sto_to_dquote
		    . comment_or_eol
                    ]

let keepalive     = [ key "keepalive" . sep
                    . [ label "ping"    . num ] . sep
		    . [ label "timeout" . num ] . comment_or_eol
                    ]

let tls_auth      = [ key "tls-auth" . sep
                    . [ label "key"       . filename     ] . sep
		    . [ label "is_client" . store /[01]/ ] . comment_or_eol
                    ]

let remote        = [ key "remote" . sep
                    . [ label "server" . filename ] . sep
		    . [ label "port"   . num      ] . comment_or_eol
		    ]

let http_proxy    = [ key "http-proxy" .
                    ( sep . [ label "server" . store /[A-Za-z0-9._-]+/ ] .
		    ( sep . [ label "port"   . num      ] )? )?
		    . comment_or_eol
		    ]

let management    = [ key "management" . sep
                    . [ label "server" . ip             ] . sep
                    . [ label "port"   . num            ] . sep
                    . [ label "pwfile" . filename       ] . comment_or_eol
                    ]


let other         = server
                  | server_bridge
		  | route
                  | push
		  | keepalive
		  | tls_auth
		  | remote
		  | http_proxy
		  | management


(************************************************************************
 *                              LENS & FILTER
 *************************************************************************)

let lns    = ( comment | empty | single | flag | other )*

let filter = (incl "/etc/openvpn/client.conf")
           . (incl "/etc/openvpn/server.conf")

let xfm = transform lns filter



