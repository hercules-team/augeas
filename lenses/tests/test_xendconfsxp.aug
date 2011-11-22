module Test_xendconfsxp =

 (* Empty file and comments *)

 test Xendconfsxp.lns get "\n" = { }

 test Xendconfsxp.lns get "# my comment\n" =
   { "#scomment" = " my comment" }

 test Xendconfsxp.lns get " \n# my comment\n" =
   { }
   { "#scomment" = " my comment" }

 test Xendconfsxp.lns get "# my comment\n\n" =
   { "#scomment" = " my comment" }
   { }

 test Xendconfsxp.lns get "# my comment\n \n" =
   { "#scomment" = " my comment" }
   { }

 test Xendconfsxp.lns get "# my comment\n#com2\n" =
   { "#scomment" = " my comment" }
   { "#scomment" = "com2" }

 test Xendconfsxp.lns get "# my comment\n\n" =
   { "#scomment" = " my comment" }
   { }

 test Xendconfsxp.lns get "\n# my comment\n" =
   { }
   { "#scomment" = " my comment" }

 test Xendconfsxp.lns get "(key value)" =
   { "key" { "item" = "value" } }

 test Xendconfsxp.lns get "(key value)\n# com\n" =
   { "key" { "item" = "value" } }
   { }
   { "#scomment" = " com" }

 test Xendconfsxp.lns get "(k2ey value)" =
   { "k2ey" { "item" = "value" } }

 test Xendconfsxp.lns get "(- value)" =
   { "-" { "item" = "value" } }

 test Xendconfsxp.lns get "(-bar value)" =
   { "-bar" { "item" = "value" } }

 test Xendconfsxp.lns get "(k-ey v-alue)\n# com\n" =
   { "k-ey" { "item" = "v-alue" } }
   { }
   { "#scomment" = " com" }

 test Xendconfsxp.lns get "(key value)" =
   { "key" { "item" = "value" } }

 test Xendconfsxp.lns get "(key value)(key2 value2)" =
   { "key"
     { "item" = "value" }
   }
   { "key2"
     { "item" = "value2" }
   }

 test Xendconfsxp.lns get "( key value )( key2 value2 )" =
   { "key"
     { "item" = "value" }
   }
   { "key2"
     { "item" = "value2" }
   }

 let source_7 = "(key value)(key2 value2)"
 test Xendconfsxp.lns get source_7 =
   { "key"
     { "item" = "value" }
   }
   { "key2"
     { "item" = "value2" }
   }

(* 2 items?  it is a key/item *)

 test Xendconfsxp.lns get "(key value)" =
   { "key"
     { "item" = "value" }
   }

 test Xendconfsxp.lns get "( key value )" =
   { "key"
     { "item" = "value" }
   }

(* 3 items?  Not allowed. 2nd item must be in ()'s *)
 test Xendconfsxp.lns get "(key value value2)" = *

(* key/list pairs. *)

(* 0 item -- TODO: implement this. *)

 test Xendconfsxp.lns get "( key ())" = *

(* 1 item *)

 test Xendconfsxp.lns get "(key (listitem1) )" =
   { "key"
     { "array"
       { "item" = "listitem1" }
     }
   }

 test Xendconfsxp.lns get "(key ( (foo bar bar bat ) ) )" =
   { "key"
     { "array"
       { "array"
         { "item" = "foo" }
         { "item" = "bar" }
         { "item" = "bar" }
         { "item" = "bat" }
       }
     }
   }

 test Xendconfsxp.lns get "(key ((foo foo foo (bar bar bar))))" =
   { "key"
     { "array"
       { "array"
         { "item" = "foo" }
         { "item" = "foo" }
         { "item" = "foo" }
         { "array"
           { "item" = "bar" }
           { "item" = "bar" }
           { "item" = "bar" }
         }
       }
     }
   }

(* 2 item *)

 test Xendconfsxp.lns get "(xen-api-server (foo bar))" =
   { "xen-api-server"
     { "array"
       { "item" = "foo" }
       { "item" = "bar" }
     }
   }

 test Xendconfsxp.lns get "( key ( value value2 ))" =
   { "key"
     { "array"
       { "item" = "value" }
       { "item" = "value2" }
     }
   }

(* 3 item *)

 test Xendconfsxp.lns get "( key ( value value2 value3 ))" =
   { "key"
     { "array"
       { "item" = "value" }
       { "item" = "value2" }
       { "item" = "value3" }
     }
   }

(* quotes *)
 test Xendconfsxp.lns get "(key \"foo\")" = { "key" { "item" = "\"foo\"" } }
 test Xendconfsxp.lns get "(key \"\")" = { "key" { "item" = "\"\"" } }
 test Xendconfsxp.lns get "( key \"foo\" )" = { "key" { "item" = "\"foo\"" } }
 test Xendconfsxp.lns get "( key \"f oo\" )" = { "key" { "item" = "\"f oo\"" } }
 test Xendconfsxp.lns get "( key \"f  oo\" )" = { "key" { "item" = "\"f  oo\"" } }
 test Xendconfsxp.lns get "(foo \"this is \\\"quoted\\\" in quotes\")" =
   { "foo" { "item" = "\"this is \\\"quoted\\\" in quotes\"" } }

 test Xendconfsxp.lns get "( key \"fo\\'\" )" = { "key" { "item" = "\"fo\\'\""} }
 test Xendconfsxp.lns get "( key \"fo\\'\" )" = { "key" { "item" = "\"fo\\'\""} }
 test Xendconfsxp.lns get "( key \"\")" = { "key" { "item" = "\"\"" } }

(* Newlines in strange places *)
 test Xendconfsxp.lns get "(\nkey\nbar\n)" = { "key" { "item" = "bar" } }
 test Xendconfsxp.lns get "(\n\n\nkey\n\n\nbar\n\n\n)" = { "key" { "item" = "bar" } }
 test Xendconfsxp.lns get "(\nkey\nbar\n)" = { "key" { "item" = "bar" } }

(* Comments in strange places *)
 test Xendconfsxp.lns get "(\nkey #aa\nbar)" =
   { "key"
     { "#comment" = "aa" }
     { "item" = "bar" }
   }

 test Xendconfsxp.lns get "(\nkey\n#aa\nbar)" =
   { "key"
     { "#comment" = "aa" }
     { "item" = "bar" }
   }

 test Xendconfsxp.lns get "(\nkey\n #aa\nbar)" =
   { "key"
     { "#comment" = "aa" }
     { "item" = "bar" }
   }

(* Comments must have 1 space before the # *)
 test Xendconfsxp.lns get "(\nkey# com\nbar\n)" = *
 test Xendconfsxp.lns get "(\nkey#aa\nbar)" = *

(* Comments may start a line *)
 test Xendconfsxp.lns get "(\nkey\n# com\nbar)" =
   { "key"
     { "#comment" = "com" }
     { "item" = "bar" }
   }
 test Xendconfsxp.lns get "(\nkey\n#aa\nbar)" =
   { "key"
     { "#comment" = "aa" }
     { "item" = "bar" }
   }

(* Sub lists *)
 test Xendconfsxp.lns get "(key ((foo foo) (bar bar)))" =
   { "key"
     { "array"
       { "array"
         { "item" = "foo" }
         { "item" = "foo" }
       }
       { "array"
         { "item" = "bar" }
         { "item" = "bar" }
       }
     }
   }


 test Xendconfsxp.lns get "(aaa ((bbb ccc ddd) (eee fff)))" =
   { "aaa"
     { "array"
       { "array"
         { "item" = "bbb" }
         { "item" = "ccc" }
         { "item" = "ddd" }
       }
       { "array"
         { "item" = "eee" }
         { "item" = "fff" }
       }
     }
   }

(* Comments in strange places *)
 test Xendconfsxp.lns get "(\nkey\nbar # bb\n)" =
   { "key"
     { "item" = "bar" }
     { "#comment" = "bb" }
   }
 test Xendconfsxp.lns get "(\nkey\nbar \n#cc\n)" =
   { "key" { "item" = "bar" }
           { "#comment" = "cc" } }
 test Xendconfsxp.lns get "(\nkey\nbar \n #cc\n)" =
   { "key" { "item" = "bar" }
           { "#comment" = "cc" } }
 test Xendconfsxp.lns get "(\nkey\nbar \n #cc\n )" =
   { "key" { "item" = "bar" }
           { "#comment" = "cc" } }
 test Xendconfsxp.lns get "(foo ((foo foo foo) (unix none)))" =
   { "foo"
     { "array"
       { "array"
         { "item" = "foo" }
         { "item" = "foo" }
         { "item" = "foo" }
       }
       { "array"
         { "item" = "unix" }
         { "item" = "none" }
       }
     }
   }

 test Xendconfsxp.lns get "(foo ((foo foo 'foo') (unix none)))" =
   { "foo"
     { "array"
       { "array"
         { "item" = "foo" }
         { "item" = "foo" }
         { "item" = "'foo'" }
       }
       { "array"
         { "item" = "unix" }
         { "item" = "none" }
       }
     }
   }

 test Xendconfsxp.lns get "(xen-api-server ((9363 pam '^localhost$ example\\.com$') (unix none)))" =
   { "xen-api-server"
     { "array"
       { "array"
         { "item" = "9363" }
         { "item" = "pam" }
         { "item" = "'^localhost$ example\.com$'" }
       }
       { "array"
         { "item" = "unix" }
         { "item" = "none" }
       }
     }
   }

 test Xendconfsxp.lns get "# -*- sh -*-\n#foo\n#bar\n\n\n(foo bar)" =
   { "#scomment" = " -*- sh -*-" }
   { "#scomment" = "foo" }
   { "#scomment" = "bar" }
   {  }
   {  }
   { "foo"
     { "item" = "bar" }
   }

(* Test whitespace before lparen *)
 test Xendconfsxp.lns get  " (network-script network-bridge)\n#\n" =
   { "network-script" { "item" = "network-bridge" } }
                      {  }
                      { "#scomment" = "" }

(* Bugs: *)

(* Note: It works if you change the last \n to \t *)
 test Xendconfsxp.lns get "(\nkey\n# com\nbar\n)" = *
