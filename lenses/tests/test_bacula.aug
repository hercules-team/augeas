module Test_Bacula =

   (* TODO: put tests *)

   test (Bacula.line Bacula.keyvalue) get "Name = kaki-sd\n" =
      {"Name" 
        {"" = "kaki-sd"}
      }

   test (Bacula.line Bacula.include) get "@/etc/foo.conf\n" =
      {"@include" = "/etc/foo.conf"}

   test (Bacula.line Bacula.keyvalue) get "Name = kaki-sd;" =
      {"Name" 
        {"" = "kaki-sd"}
      }

   test (Bacula.line Bacula.include) get "@foobar  ;" =
      {"@include" = "foobar"}

   test Bacula.lns get "Storage {\n   Name = kaki-sd\n}" =
      {"@block" = "Storage"
        {"Name" 
          {"" = "kaki-sd"}
        }
      }

   (* value can have quotes *)
   test Bacula.lns get "Storage {\n   Name = \"kaki-sd\"\n}" =
      {"@block" = "Storage"
        {"Name" 
          {"\"" = "kaki-sd"}
        }
      }

   (* whitespace in key *)
   test Bacula.lns get "Storage {\n   Pid Directory = kaki sd\n}" =
      {"@block" = "Storage"
         {"Pid Directory"
          {"" = "kaki sd"}
         }
      }

   (* one char value *)
   test Bacula.lns get "Storage {\n   Maximum Concurrent Jobs = 1\n}" =
      {"@block" = "Storage"
         {"Maximum Concurrent Jobs"
          {"" = "1"}
         }
      }

   (* semicolon *)
   test Bacula.lns get "Storage {\n   Name = kaki-sd;\n}" =
      {"@block" = "Storage"
        {"Name" 
          {"" = "kaki-sd"}
        }
      }

   (* inline comment *)
   test Bacula.lns get "Storage {\n   Name = kaki-sd         # just a comment\n}" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
          { "#comment" = "just a comment"}
        }
      }

   (* comment as part of directive *)
   test Bacula.lns get "Storage {\n   Name = kaki-sd\n # just a comment\n}" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
        { "#comment" = "just a comment"}
      }

   (* comment after } *)
   test Bacula.lns get "Storage {\n   Name = kaki-sd\n}\n # just a comment\n" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
      }
      { }
      { "#comment" = "just a comment"}

   (* multiple values *)
   test Bacula.lns get "Storage {\n  Name = kaki-sd\nFoo = moo\n}" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
        {"Foo" 
          { "" = "moo" }
        }
      }

   (* toplevel key/value for include files *)
   test Bacula.lns get "Name = kaki-sd\nFoo = moo\n" =
      {"Name" 
        { "" = "kaki-sd" }
      }
      {"Foo" 
        { "" = "moo" }
      }

   (* escaping quotes in value *)
   test Bacula.lns get "Storage {\nName = \"foo \\" bar\"\n}" =
      {"@block" = "Storage"
        {"Name" 
          { "\"" = "foo \\" bar" }
        }
      }

   (* newline comment *)
   test Bacula.lns get "Storage {\n  Name = kaki-sd\n# just a comment\n}" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
        {"#comment" = "just a comment" }
      }

   (* include statements *)
   test Bacula.lns get "Storage {\n  @/etc/foo.conf\n}" =
      {"@block" = "Storage"
         {"@include" = "/etc/foo.conf"}
      }

   test Bacula.lns get "Storage {\n   Name = kaki-sd}" =
      {"@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
      }

   test Bacula.lns get "FileSet { Include { signature = SHA1 } }" =
   { "@block" = "FileSet"
       { "@block" = "Include"
         { "signature" 
           { "" = "SHA1" }
         }
       }
   }
   
   test Bacula.lns get "FileSet {
  Name = \"DefaultSet\"
  Include {
    Options {
      signature = SHA1
      noatime = yes
    }
    File = /etc
  }
}" =
      {"@block" = "FileSet"
         { "Name" 
           { "\"" = "DefaultSet" }
         }
         {"@block" = "Include"
            {"@block" = "Options"
              { "signature" 
                { "" = "SHA1" }
              }
              { "noatime" 
                { "" = "yes" }
              }
            }
            { }
            { "File" 
              { "" = "/etc" }
            }
         }
      }

   (* include top level statements *)
   test Bacula.lns get "@/etc/foo.conf\n" =
      {"@include" = "/etc/foo.conf"}

   (* Blocks can follow each other without \n *)
   test Bacula.lns get "Storage{Name = kaki sd}Storage{Name = kaki-sd}" =
   { "@block" = "Storage"
        {"Name" 
          { "" = "kaki sd" }
        }
   }
   { "@block" = "Storage"
        {"Name" 
          { "" = "kaki-sd" }
        }
   }

   (* recursive directives *)
   test Bacula.lns get "FileSet { Include { signature = SHA1 } }" =
   { "@block" = "FileSet"
       { "@block" = "Include"
         { "signature" 
           { "" = "SHA1" }
         }
       }
   }

   (* quotes followed by .. stuff *)
   test Bacula.lns get "Storage {\nappend = \"/var/lib/bacula/log\" = all, !skipped}" =
     { "@block" = "Storage"
         { "append"
           { "\"" = "/var/lib/bacula/log"
             { = " = all, !skipped" }
           }
         }
     }

   (* TODO: \ can break lines into multiple *)

   (* include top with inline shell *)
   test Bacula.lns get "@|\"sh -c 'cat /etc/bacula/clients/*.conf'\";" =
      {"@include" = "|\"sh -c 'cat /etc/bacula/clients/*.conf'\""}

   let conf ="Storage {
  Name = kaki-sd
  SDPort = 9103                  # Director's port      
  WorkingDirectory = \"/var/lib/bacula\"
  Pid Directory = \"/var/run/bacula\"
  Maximum Concurrent Jobs = 20
  SDAddress = 127.0.0.1
}

Director {
  Name = kaki-dir
  Password = \"FOObar\"
}

Director {
  Name = kaki-mon
  Password = \"fooBAR\"
  Monitor = yes
}

Device {
  Name = FileStorage
  Media Type = File
  Archive Device = /tmp/
  LabelMedia = yes;                   # lets Bacula label unlabeled media
  Random Access = Yes;
}

Messages {
  Name = Standard
  director = kaki-dir = all
}

# just a comment
"

   test Bacula.lns get conf =
      { "@block" = "Storage"
         { "Name"
            { "" = "kaki-sd" }
         }
         { "SDPort"
            { "" = "9103" }
            { "#comment" = "Director's port" }
         }
         { "WorkingDirectory"
            { "\"" = "/var/lib/bacula" }
         }
         { "Pid Directory"
            { "\"" = "/var/run/bacula" }
         }
         { "Maximum Concurrent Jobs"
            { "" = "20" }
         }
         { "SDAddress"
            { "" = "127.0.0.1" }
         }
      } 
      { }
      { }
      { "@block" = "Director"
         { "Name"
            { "" = "kaki-dir" }
         }
         { "Password"
            { "\"" = "FOObar" }
         }
      }
      { }
      { }
      { "@block" = "Director"
         { "Name"
            { "" = "kaki-mon" }
         }
         { "Password"
            { "\"" = "fooBAR" }
         }
         { "Monitor"
            { "" = "yes" }
         }
      }
      { }
      { }
      { "@block" = "Device"
         { "Name"
            { "" = "FileStorage" }
         }
         { "Media Type"
            { "" = "File" }
         }
         { "Archive Device"
            { "" = "/tmp/" }
         }
         { "LabelMedia"
            { "" = "yes" }
         }
         { "#comment" = "lets Bacula label unlabeled media" }
         { "Random Access"
            { "" = "Yes" }
         }
      }
      { }
      { }
      { "@block" = "Messages"
         { "Name"
            { "" = "Standard" }
         }
         { "director"
            { "" = "kaki-dir = all" }
         }
      }
      { }
      { }
      { "#comment" = "just a comment" }
