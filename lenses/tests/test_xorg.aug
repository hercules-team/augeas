(* Tests for the Xorg module *)

module Test_xorg =

  let conf = "
# xorg.conf

Section \"ServerLayout\"
        Identifier     \"single head configuration\"
        Screen      0  \"Screen0\" 0 0
        InputDevice    \"Generic Keyboard\" \"CoreKeyboard\"
EndSection

Section \"InputDevice\"
	Identifier	\"Generic Keyboard\"
        # that's a driver
	Driver		\"kbd\"
	Option		\"XkbOptions\"	\"lv3:ralt_switch\"
EndSection

Section \"Device\"
	Identifier	\"Configured Video Device\"
	Option 		\"MonitorLayout\" \"LVDS,VGA\"
	VideoRam	229376
        Option          \"NoAccel\"
        Screen          0
EndSection

Section \"Screen\"
        Identifier \"Screen0\"
        Device     \"Configured Video Device\"
        DefaultDepth     24
        SubSection \"Display\"
                Viewport   0 0
                Depth     24
                Modes    \"1280x1024\" \"1280x960\" \"1280x800\"
        EndSubSection
EndSection

Section \"Module\"
          SubSection \"extmod\"
                   Option  \"omit XFree86-DGA\"
          EndSubSection
EndSection
"

  test Xorg.lns get conf =
     { }
     { "#comment" = "xorg.conf" }
     { }
     { "ServerLayout"
        { "Identifier" = "single head configuration" }
        { "Screen"     = "Screen0"
           { "num"      = "0" }
           { "position" = "0 0" } }
        { "InputDevice" = "Generic Keyboard"
           { "option"   = "CoreKeyboard" } } }
     { }
     { "InputDevice"
        { "Identifier" = "Generic Keyboard" }
        { "#comment"   = "that's a driver" }
        { "Driver"     = "kbd" }
        { "Option"     = "XkbOptions"
             { "value"  = "lv3:ralt_switch" } } }
     { }
     { "Device"
        { "Identifier" = "Configured Video Device" }
        { "Option"     = "MonitorLayout"
             { "value"  = "LVDS,VGA" } }
        { "VideoRam"   = "229376" }
        { "Option"     = "NoAccel" } 
        { "Screen"
          { "num" = "0" } } }
     { }
     { "Screen"
        { "Identifier" = "Screen0" }
        { "Device"     = "Configured Video Device" }
        { "DefaultDepth" = "24" }
        { "Display"
           { "ViewPort"
              { "x" = "0" }
              { "y" = "0" } }
           { "Depth"    = "24" }
           { "Modes"
              { "mode" = "1280x1024" }
              { "mode" = "1280x960" }
              { "mode" = "1280x800" } } } }
     { }
     { "Module"
       { "extmod"
           { "Option" = "omit XFree86-DGA" } } }
