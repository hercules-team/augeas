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
      {}
      { "safe_mode" = "Off" }
      { "section" = "PHP"
         { "#comment" = "Enable the PHP scripting language engine under Apache." }
	 { "engine"  = "On" }
	 {}
	 { "#comment" = "Enable compatibility mode with Zend Engine 1 (PHP 4.x)" }
	 { "zend.ze1_compatibility_mode" = "Off" }
	 { "unserialize_callback_func" }
	 { "date.default_latitude" = "31.7667" }
	 {} }
      { "section" = "sqlite"
         { "sqlite.assoc_case" = "0" } }
