module Test_DevfsRules =

  let manpage_example = "[localrules=10]
add path 'da*s*' mode 0660 group usb
"

  test DevfsRules.lns get manpage_example =
  { "localrules" { "id" = "10" }
      { "1" = "add path 'da*s*' mode 0660 group usb" } }


  let example = "[devfsrules_jail_unhide_usb_printer_and_scanner=30]
add include $devfsrules_hide_all
add include $devfsrules_unhide_basic
add include $devfsrules_unhide_login
add path 'ulpt*' mode 0660 group printscan unhide 
add path 'unlpt*' mode 0660 group printscan unhide
add path 'ugen2.8' mode 0660 group printscan unhide  # Scanner (ugen2.8 is a symlink to usb/2.8.0)
add path usb unhide
add path usbctl unhide
add path 'usb/2.8.0' mode 0660 group printscan unhide

[devfsrules_jail_unhide_usb_scanner_only=30]
add include $devfsrules_hide_all
add include $devfsrules_unhide_basic
add include $devfsrules_unhide_login
add path 'ugen2.8' mode 0660 group scan unhide  # Scanner
add path usb unhide
add path usbctl unhide
add path 'usb/2.8.0' mode 0660 group scan unhide
"

  test DevfsRules.lns get example =
    { "devfsrules_jail_unhide_usb_printer_and_scanner" { "id" = "30" }
      { "1" = "add include $devfsrules_hide_all" }
      { "2" = "add include $devfsrules_unhide_basic" }
      { "3" = "add include $devfsrules_unhide_login" }
      { "4" = "add path 'ulpt*' mode 0660 group printscan unhide" }
      { "5" = "add path 'unlpt*' mode 0660 group printscan unhide" }
      { "6" = "add path 'ugen2.8' mode 0660 group printscan unhide"
        { "#comment" = "Scanner (ugen2.8 is a symlink to usb/2.8.0)" }
      }
      { "7" = "add path usb unhide" }
      { "8" = "add path usbctl unhide" }
      { "9" = "add path 'usb/2.8.0' mode 0660 group printscan unhide" }
      {  }
    }
    { "devfsrules_jail_unhide_usb_scanner_only" { "id" = "30" }
      { "1" = "add include $devfsrules_hide_all" }
      { "2" = "add include $devfsrules_unhide_basic" }
      { "3" = "add include $devfsrules_unhide_login" }
      { "4" = "add path 'ugen2.8' mode 0660 group scan unhide"
        { "#comment" = "Scanner" }
      }
      { "5" = "add path usb unhide" }
      { "6" = "add path usbctl unhide" }
      { "7" = "add path 'usb/2.8.0' mode 0660 group scan unhide" }
    }

