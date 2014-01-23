(* Test for shell list handling lens *)
module Test_shellvars_list =

  let list_vals = "# Some comment
MODULES_LOADED_ON_BOOT=\"ipv6 sunrpc\"

DEFAULT_APPEND=\"showopts noresume console=tty0 console=ttyS0,115200n8 ro\"

LOADER_TYPE=\"grub\"
"

  test Shellvars_list.lns get list_vals =
    { "#comment" = "Some comment" }
    { "MODULES_LOADED_ON_BOOT"
      { "quote" = "\"" }
      { "value" = "ipv6" }
      { "value" = "sunrpc" } }
    {  }
    { "DEFAULT_APPEND"
      { "quote" = "\"" }
      { "value" = "showopts" }
      { "value" = "noresume" }
      { "value" = "console=tty0" }
      { "value" = "console=ttyS0,115200n8" }
      { "value" = "ro" } }
    {  }
    { "LOADER_TYPE"
      { "quote" = "\"" }
      { "value" = "grub" } }


  (* append a value *)
  test Shellvars_list.lns put "VAR=\"test1\t  \ntest2\"\n" after
    set "VAR/value[last()+1]" "test3"
    = "VAR=\"test1\t  \ntest2 test3\"\n"

  (* in double quoted lists, single quotes and escaped values are allowed *)
  test Shellvars_list.lns get "VAR=\"test'1 test2 a\ \\\"longer\\\"\ test\"\n" =
    { "VAR"
      { "quote" = "\"" }
      { "value" = "test'1" }
      { "value" = "test2" }
      { "value" = "a\ \\\"longer\\\"\ test" } }

  (* add new value, delete one and append something *)
  test Shellvars_list.lns put list_vals after
    set "FAILSAVE_APPEND/quote" "\"" ;
    set "FAILSAVE_APPEND/value[last()+1]" "console=ttyS0" ;
    rm "LOADER_TYPE" ;
    rm "MODULES_LOADED_ON_BOOT/value[1]" ;
    set "DEFAULT_APPEND/value[last()+1]" "teststring"
    = "# Some comment
MODULES_LOADED_ON_BOOT=\"sunrpc\"

DEFAULT_APPEND=\"showopts noresume console=tty0 console=ttyS0,115200n8 ro teststring\"

FAILSAVE_APPEND=\"console=ttyS0\"
"

  (* test of single quotes (leading/trailing whitespaces are kept *)
  (* leading/trailing) *)
  test Shellvars_list.lns put "VAR=' \t test1\t  \ntest2  '\n" after
    set "VAR/value[last()+1]" "test3"
    = "VAR=' \t test1\t  \ntest2 test3  '\n"

  (* change quotes (leading/trailing whitespaces are lost *)
  test Shellvars_list.lns put "VAR=' \t test1\t  \ntest2  '\n" after
    set "VAR/quote" "\""
    = "VAR=\"test1\t  \ntest2\"\n"

  (* double quotes are allowed in single quoted lists *)
  test Shellvars_list.lns get "VAR='test\"1 test2'\n" =
    { "VAR"
      { "quote" = "'" }
      { "value" = "test\"1" }
      { "value" = "test2" } }

  (* emtpy list with quotes *)
  test Shellvars_list.lns get "VAR=''\n" =
    { "VAR"
      { "quote" = "'" } }

  (* unquoted value *)
  test Shellvars_list.lns get "VAR=test\n" =
    { "VAR"
      { "quote" = "" }
      { "value" = "test" } }

  (* uquoted value with escaped space etc. *)
  test Shellvars_list.lns get "VAR=a\\ \\\"long\\\"\\ test\n" =
    { "VAR"
      { "quote" = "" }
      { "value" = "a\\ \\\"long\\\"\\ test" } }

  (* append to unquoted value *)
  test Shellvars_list.lns put "VAR=test1\n" after
    set "VAR/quote" "\"";
    set "VAR/value[last()+1]" "test2"
    = "VAR=\"test1 test2\"\n"

  (* empty entry *)
  test Shellvars_list.lns get "VAR=\n" =
    { "VAR"
      { "quote" = "" } }

  (* set value w/o quotes to empty value... *)
  test Shellvars_list.lns put "VAR=\n" after
    set "VAR/value[last()+1]" "test"
    = "VAR=test\n"

  (* ... or no value *)
  test Shellvars_list.lns put "" after
    set "VAR/quote" "";
    set "VAR/value[1]" "test"
    = "VAR=test\n"

  (* Ticket #368 - backticks *)
  test Shellvars_list.lns get "GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`\n" =
    { "GRUB_DISTRIBUTOR"
      { "quote" = "" }
      { "value" = "`lsb_release -i -s 2> /dev/null || echo Debian`" } }

  (* Test: Shellvars_list.lns
       Ticket #342: end-of-line comments *)
  test Shellvars_list.lns get "service_ping=\"ping/icmp\" #ping\n" =
    { "service_ping"
      { "quote" = "\"" }
      { "value" = "ping/icmp" }
      { "#comment" = "ping" } }

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
