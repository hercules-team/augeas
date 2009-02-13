(* Tests for the Xorg module *)

module Test_xorg =

  let conf = "
# xorg.conf

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
EndSection
"

  test Xorg.lns get conf =
     { }
     { "#comment" = "xorg.conf" }
     { }
     { "InputDevice"
        { "Identifier" = "\"Generic Keyboard\"" }
        { "#comment"   = "that's a driver" }
        { "Driver"     = "\"kbd\"" }
        { "Option"     = "\"XkbOptions\""
             { "value"  = "\"lv3:ralt_switch\"" } } }
     { }
     { "Device"
        { "Identifier" = "\"Configured Video Device\"" }
        { "Option"     = "\"MonitorLayout\""
             { "value"  = "\"LVDS,VGA\"" } }
        { "VideoRam"   = "229376" } }

