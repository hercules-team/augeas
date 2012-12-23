module Test_xinetd =

let eol_ws = "defaults \t \n{\n  enabled = cvs echo  \n}\n\n"

let cvs = "# default: off
# description: The CVS service can record the history of your source
#              files. CVS stores all the versions of a file in a single
#              file in a clever way that only stores the differences
#              between versions.
service cvspserver
{
        disable                 = yes
        port                    = 2401
        socket_type             = stream
        protocol                = tcp
        wait                    = no
        user                    = root
        passenv                 = PATH
        server                  = /usr/bin/cvs
        env                    -= HOME=/var/cvs
        server_args             = -f --allow-root=/var/cvs pserver
#       bind                    = 127.0.0.1
        log_on_failure         += HOST
        FLAGS                   = IPv6 IPv4
}
"

let lst_add = "service svc_add
{
   log_on_failure += HOST
}
"

test Xinetd.lns get eol_ws =
  { "defaults" { "enabled"
                 { "value" = "cvs" }
                 { "value" = "echo" } } }
  {}

test Xinetd.lns put eol_ws after rm "/defaults/enabled/value[last()]" =
  "defaults \t \n{\n  enabled = cvs  \n}\n\n"

test Xinetd.lns get cvs =
  { "#comment" = "default: off" }
  { "#comment" = "description: The CVS service can record the history of your source" }
  { "#comment" = "files. CVS stores all the versions of a file in a single" }
  { "#comment" = "file in a clever way that only stores the differences" }
  { "#comment" = "between versions." }
  { "service" = "cvspserver"
      { "disable" = "yes" }
      { "port" = "2401" }
      { "socket_type" = "stream" }
      { "protocol" = "tcp" }
      { "wait" = "no" }
      { "user" = "root" }
      { "passenv" { "value" = "PATH" } }
      { "server" = "/usr/bin/cvs" }
      { "env" { "del" } { "value" = "HOME=/var/cvs" } }
      { "server_args"
          { "value" = "-f" }
          { "value" = "--allow-root=/var/cvs" }
          { "value" = "pserver" } }
      { "#comment" = "bind                    = 127.0.0.1" }
      { "log_on_failure" { "add" } { "value" = "HOST" } }
      { "FLAGS"
          { "value" = "IPv6" }
          { "value" = "IPv4" } } }

(* Switch the '+=' to a simple '=' *)
test Xinetd.lns put lst_add after rm "/service/log_on_failure/add" =
  "service svc_add\n{\n   log_on_failure = HOST\n}\n"

test Xinetd.lns put "" after
  set "/service" "svc";
  set "/service/instances" "UNLIMITED" = "service svc
{
\tinstances = UNLIMITED
}
"

(* Support missing values in lists *)
test Xinetd.lns get "service check_mk\n{\n  log_on_success =\n  server_args=\n}\n" =
  { "service" = "check_mk"
    { "log_on_success" }
    { "server_args" }
  }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
