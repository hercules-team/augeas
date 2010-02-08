module Test_cobblermodules =

   let conf = "
[serializes]
settings = serializer_catalog

[authentication]
modules = auth_denyall
"

   test CobblerModules.lns get conf =
      {}
      { "serializes"
         { "settings" = "serializer_catalog" }
         {} }
      { "authentication"
         { "modules" = "auth_denyall" } }

    test CobblerModules.lns put conf after
       set "serializes/distro" "serializer_catalog";
       set "serializes/repo" "serializer_catalog"
    = "
[serializes]
settings = serializer_catalog

distro=serializer_catalog
repo=serializer_catalog
[authentication]
modules = auth_denyall
"
