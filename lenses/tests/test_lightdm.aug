(* 
Module: Test_Lightdm
   Module to test Lightdm module for Augeas

Author: David Salmen <dsalmen@dsalmen.com>

About: License
   This file is licenced under the LGPL v2+, like the rest of Augeas.
*)

module Test_lightdm =

    let conf_lightdm = "
[SeatDefaults]
greeter-session=unity-greeter
user-session=ubuntu
"

   test Lightdm.lns get conf_lightdm =
      {}
      { "SeatDefaults"
         { "greeter-session" = "unity-greeter" }
         { "user-session" = "ubuntu" }
      }

    test Lightdm.lns put conf_lightdm after
       set "SeatDefaults/allow-guest" "false"
    = "
[SeatDefaults]
greeter-session=unity-greeter
user-session=ubuntu
allow-guest=false
"

    test Lightdm.lns put conf_lightdm after
       set "SeatDefaults/allow-guest" "true"
    = "
[SeatDefaults]
greeter-session=unity-greeter
user-session=ubuntu
allow-guest=true
"

    let conf_unity_greeter = "
#
# background = Background file to use, either an image path or a color (e.g. #772953)
# logo = Logo file to use
# theme-name = GTK+ theme to use
# font-name = Font to use
# xft-antialias = Whether to antialias Xft fonts (true or false)
# xft-dpi = Resolution for Xft in dots per inch (e.g. 96)
# xft-hintstyle = What degree of hinting to use (hintnone, hintslight, hintmedium, or hintfull)
# xft-rgba = Type of subpixel antialiasing (none, rgb, bgr, vrgb or vbgr)
#
[greeter]
background=/usr/share/backgrounds/warty-final-ubuntu.png
logo=/usr/share/unity-greeter/logo.png
theme-name=Ambiance
icon-theme-name=ubuntu-mono-dark
font-name=Ubuntu 11
xft-antialias=true
xft-dpi=96
xft-hintstyle=hintslight
xft-rgba=rgb
" 

    test Lightdm.lns get conf_unity_greeter =
        {}
        {}
        { "#comment" = "background = Background file to use, either an image path or a color (e.g. #772953)" }
        { "#comment" = "logo = Logo file to use" }
        { "#comment" = "theme-name = GTK+ theme to use" }
        { "#comment" = "font-name = Font to use" }
        { "#comment" = "xft-antialias = Whether to antialias Xft fonts (true or false)" }
        { "#comment" = "xft-dpi = Resolution for Xft in dots per inch (e.g. 96)" }
        { "#comment" = "xft-hintstyle = What degree of hinting to use (hintnone, hintslight, hintmedium, or hintfull)" }
        { "#comment" = "xft-rgba = Type of subpixel antialiasing (none, rgb, bgr, vrgb or vbgr)" }
        {}
        { "greeter"
            { "background" = "/usr/share/backgrounds/warty-final-ubuntu.png" }
            { "logo" = "/usr/share/unity-greeter/logo.png" }
            { "theme-name" = "Ambiance" }
            { "icon-theme-name" = "ubuntu-mono-dark" }
            { "font-name" = "Ubuntu 11" }
            { "xft-antialias" = "true" }
            { "xft-dpi" = "96" }
            { "xft-hintstyle" = "hintslight" }
            { "xft-rgba" = "rgb" }
        }

    let conf_users = "
#
# User accounts configuration
#
# NOTE: If you have AccountsService installed on your system, then LightDM will
# use this instead and these settings will be ignored
#
# minimum-uid = Minimum UID required to be shown in greeter
# hidden-users = Users that are not shown to the user
# hidden-shells = Shells that indicate a user cannot login
#
[UserAccounts]
minimum-uid=500
hidden-users=nobody nobody4 noaccess
hidden-shells=/bin/false /usr/sbin/nologin
"

    test Lightdm.lns get conf_users =
        {}
        {}
        { "#comment" = "User accounts configuration" }
        {}
        { "#comment" = "NOTE: If you have AccountsService installed on your system, then LightDM will" }
        { "#comment" = "use this instead and these settings will be ignored" }
        {}
        { "#comment" = "minimum-uid = Minimum UID required to be shown in greeter" }
        { "#comment" = "hidden-users = Users that are not shown to the user" }
        { "#comment" = "hidden-shells = Shells that indicate a user cannot login" }
        {}
        { "UserAccounts"
            { "minimum-uid" = "500" }
            { "hidden-users" = "nobody nobody4 noaccess" }
            { "hidden-shells" = "/bin/false /usr/sbin/nologin" }
        }

