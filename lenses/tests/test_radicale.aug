module Test_radicale =

   let conf = "
[server]

[encoding]

[well-known]

[auth]

[git]

[rights]

[storage]

[logging]

[headers]

"

   test Radicale.lns get conf =
      {}
      { "server"
         {} }
      { "encoding"
         {} }
      { "well-known"
         {} }
      { "auth"
         {} }
      { "git"
         {} }
      { "rights"
         {} }
      { "storage"
         {} }
      { "logging"
         {} }
      { "headers"
         {} }

    test Radicale.lns put conf after
       set "server/hosts" "127.0.0.1:5232, [::1]:5232";
       set "server/base_prefix" "/radicale/";
       set "well-known/caldav" "/radicale/%(user)s/caldav/";
       set "well-known/cardav" "/radicale/%(user)s/carddav/";
       set "auth/type" "remote_user";
       set "rights/type" "owner_only"
    = "
[server]

hosts=127.0.0.1:5232, [::1]:5232
base_prefix=/radicale/
[encoding]

[well-known]

caldav=/radicale/%(user)s/caldav/
cardav=/radicale/%(user)s/carddav/
[auth]

type=remote_user
[git]

[rights]

type=owner_only
[storage]

[logging]

[headers]

"
