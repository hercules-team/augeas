module Test_gdm =

   let conf = "[daemon]
# Automatic login, if true the first attached screen will automatically logged
# in as user as set with AutomaticLogin key.
AutomaticLoginEnable=false
AutomaticLogin=

[server]
0=Standard device=/dev/console
"

   test Gdm.lns get conf =
      { "daemon"
         { "#comment" = "Automatic login, if true the first attached screen will automatically logged" }
         { "#comment" = "in as user as set with AutomaticLogin key." }
	 { "AutomaticLoginEnable"  = "false" }
	 { "AutomaticLogin" }
         {} }
      { "server"
         { "0" = "Standard device=/dev/console" } }
