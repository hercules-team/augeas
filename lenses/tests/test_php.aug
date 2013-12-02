module Test_php =

let conf = "
safe_mode = Off
[PHP]
; Enable the PHP scripting language engine under Apache.
engine = On

; Enable compatibility mode with Zend Engine 1 (PHP 4.x)
zend.ze1_compatibility_mode = Off
 unserialize_callback_func=
date.default_latitude = 31.7667

[sqlite]
sqlite.assoc_case = 0
"


test PHP.lns get conf =
  { ".anon"
     {}
     { "safe_mode" = "Off" } }
  { "PHP"
     { "#comment" = "Enable the PHP scripting language engine under Apache." }
     { "engine"  = "On" }
     {}
     { "#comment" = "Enable compatibility mode with Zend Engine 1 (PHP 4.x)" }
     { "zend.ze1_compatibility_mode" = "Off" }
     { "unserialize_callback_func" }
     { "date.default_latitude" = "31.7667" }
     {} }
  { "sqlite"
     { "sqlite.assoc_case" = "0" } }

test PHP.lns put conf after rm "noop" = conf


test PHP.lns get ";\n" = { ".anon" {} }

(* Section titles can have spaces *)
test PHP.lns get "[mail function]\n" =  { "mail function" }

(* Keys can be lower and upper case *)
test PHP.lns get "[fake]
SMTP = localhost
mixed_KEY = 25
" = 
 { "fake"
     { "SMTP" = "localhost" }
     { "mixed_KEY" = "25" } }

(* Ticket #243 *)
test PHP.lns get "session.save_path = \"3;/var/lib/php5\"\n" =
   { ".anon"
      { "session.save_path" = "3;/var/lib/php5" } }

(* GH issue #35
   php-fpm syntax *)
test PHP.lns get "php_admin_flag[log_errors] = on\n" =
   { ".anon"
     { "php_admin_flag[log_errors]" = "on" } }
