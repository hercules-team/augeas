module Test_pythonpaste =

   let conf = "
#blah blah
[main]
pipeline = hello

[composite:main]
use = egg:Paste#urlmap
/v2.0 = public_api
/: public_version_api
"

   test PythonPaste.lns get conf =
      { }
      { "#comment" = "blah blah" }
      { "main"
         { "pipeline" = "hello" }
         { }
      }
      { "composite:main"
         { "use" = "egg:Paste#urlmap" }
         { "1" = "/v2.0 = public_api" }
         { "2" = "/: public_version_api" }
      }


    test PythonPaste.lns put conf after
       set "main/pipeline" "goodbye";
       set "composite:main/3" "/v3: a_new_api_version"
    = "
#blah blah
[main]
pipeline = goodbye

[composite:main]
use = egg:Paste#urlmap
/v2.0 = public_api
/: public_version_api
/v3: a_new_api_version
"
