(*
Module: Test_Systemd
  Provides unit tests and examples for the <Systemd> lens.
*)

module Test_Systemd =

(* Variable: desc *)
let desc = "[Unit]
Description=RPC
Description=RPC bind service
Description=RPC bind\\
service
Description= Resets System Activity Logs
"
(* Test: Systemd.lns *)
test Systemd.lns get desc =
  { "Unit"
    { "Description"
      { "value" = "RPC" }
    }
    { "Description"
      { "value" = "RPC bind service" }
    }
    { "Description"
      { "value" = "RPC bind\
service" }
    }
    { "Description"
      { "value" = "Resets System Activity Logs" }
    }
  }

(* Variable: multi *)
let multi = "[Unit]
After=syslog.target network.target
Also=canberra-system-shutdown.service canberra-system-shutdown-reboot.service
Before=sysinit.target shutdown.target
CapabilityBoundingSet=CAP_SYS_ADMIN CAP_SETUID CAP_SETGID
Conflicts=emergency.service emergency.target
ControlGroup=%R/user/%I/shared cpu:/
ListenNetlink=kobject-uevent 1
Requires=shutdown.target umount.target final.target
Sockets=udev-control.socket udev-kernel.socket
WantedBy=halt.target poweroff.target
Wants=local-fs.target swap.target
Wants=local-fs.target \
swap.target
Wants=local-fs.target\
swap.target
Wants= local-fs.target
"
(* Test: Systemd.lns *)
test Systemd.lns get multi =
  { "Unit"
    { "After"
      { "value" = "syslog.target" }
      { "value" = "network.target" }
    }
    { "Also"
      { "value" = "canberra-system-shutdown.service" }
      { "value" = "canberra-system-shutdown-reboot.service" }
    }
    { "Before"
      { "value" = "sysinit.target" }
      { "value" = "shutdown.target" }
    }
    { "CapabilityBoundingSet"
      { "value" = "CAP_SYS_ADMIN" }
      { "value" = "CAP_SETUID" }
      { "value" = "CAP_SETGID" }
    }
    { "Conflicts"
      { "value" = "emergency.service" }
      { "value" = "emergency.target" }
    }
    { "ControlGroup"
      { "value" = "%R/user/%I/shared" }
      { "value" = "cpu:/" }
    }
    { "ListenNetlink"
      { "value" = "kobject-uevent" }
      { "value" = "1" }
    }
    { "Requires"
      { "value" = "shutdown.target" }
      { "value" = "umount.target" }
      { "value" = "final.target" }
    }
    { "Sockets"
      { "value" = "udev-control.socket" }
      { "value" = "udev-kernel.socket" }
    }
    { "WantedBy"
      { "value" = "halt.target" }
      { "value" = "poweroff.target" }
    }
    { "Wants"
      { "value" = "local-fs.target" }
      { "value" = "swap.target" }
    }
    { "Wants"
      { "value" = "local-fs.target" }
      { "value" = "swap.target" }
    }
    { "Wants"
      { "value" = "local-fs.target" }
      { "value" = "swap.target" }
    }
    { "Wants"
      { "value" = "local-fs.target" }
    }
  }

(* Variable: exec *)
let exec = "[Service]
ExecStart=/bin/ls
ExecReload=/bin/kill -USR1 $MAINPID
ExecStart=/sbin/rpcbind -w
ExecStartPost=/bin/systemctl disable firstboot-graphical.service firstboot-text.service
ExecStartPre=/sbin/modprobe -qa $SUPPORTED_DRIVERS
ExecStop=/usr/sbin/aiccu stop
ExecStopPost=-/bin/systemctl poweroff
ExecStopPost=@/bin/systemctl poweroff
ExecStopPost=-@/bin/systemctl poweroff
ExecStopPost=/bin/systemctl\
poweroff
"
(* Test: Systemd.lns *)
test Systemd.lns get exec =
  { "Service"
    { "ExecStart"
      { "command" = "/bin/ls" }
    }
    { "ExecReload"
      { "command" = "/bin/kill" }
      { "arguments"
        { "1" = "-USR1" }
        { "2" = "$MAINPID" }
      }
    }
    { "ExecStart"
      { "command" = "/sbin/rpcbind" }
      { "arguments"
        { "1" = "-w" }
      }
    }
    { "ExecStartPost"
      { "command" = "/bin/systemctl" }
      { "arguments"
        { "1" = "disable" }
        { "2" = "firstboot-graphical.service" }
        { "3" = "firstboot-text.service" }
      }
    }
    { "ExecStartPre"
      { "command" = "/sbin/modprobe" }
      { "arguments"
        { "1" = "-qa" }
        { "2" = "$SUPPORTED_DRIVERS" }
      }
    }
    { "ExecStop"
      { "command" = "/usr/sbin/aiccu" }
      { "arguments"
        { "1" = "stop" }
      }
    }
    { "ExecStopPost"
      { "ignoreexit" }
      { "command" = "/bin/systemctl" }
      { "arguments"
        { "1" = "poweroff" }
      }
    }
    { "ExecStopPost"
      { "arg0" }
      { "command" = "/bin/systemctl" }
      { "arguments"
        { "1" = "poweroff" }
      }
    }
    { "ExecStopPost"
      { "ignoreexit" }
      { "arg0" }
      { "command" = "/bin/systemctl" }
      { "arguments"
        { "1" = "poweroff" }
      }
    }
    { "ExecStopPost"
      { "command" = "/bin/systemctl" }
      { "arguments"
        { "1" = "poweroff" }
      }
    }
  }

(* Variable: env *)
let env = "[Service]
Environment=LANG=C
Environment=LANG=C FOO=BAR
Environment=LANG= LANGUAGE= LC_CTYPE= LC_NUMERIC= LC_TIME= LC_COLLATE= LC_MONETARY= LC_MESSAGES= LC_PAPER= LC_NAME= LC_ADDRESS= LC_TELEPHONE= LC_MEASUREMENT= LC_IDENTIFICATION=
Environment=LANG=C\
FOO=BAR
"
(* Test: Systemd.lns *)
test Systemd.lns get env =
  { "Service"
    { "Environment"
      { "LANG" = "C" }
    }
    { "Environment"
      { "LANG" = "C" }
      { "FOO" = "BAR" }
    }
    { "Environment"
      { "LANG" }
      { "LANGUAGE" }
      { "LC_CTYPE" }
      { "LC_NUMERIC" }
      { "LC_TIME" }
      { "LC_COLLATE" }
      { "LC_MONETARY" }
      { "LC_MESSAGES" }
      { "LC_PAPER" }
      { "LC_NAME" }
      { "LC_ADDRESS" }
      { "LC_TELEPHONE" }
      { "LC_MEASUREMENT" }
      { "LC_IDENTIFICATION" }
    }
    { "Environment"
      { "LANG" = "C" }
      { "FOO" = "BAR" }
    }
  }

(* Variable: unit *)
let unit = "#  This file is part of systemd.
#

# See systemd.special(7) for details

.include /etc/example

[Unit]
Description=Locale Service
# Add another file
.include /etc/example

[Service]
ExecStart=/lib/systemd/systemd-localed
Type=dbus
BusName=org.freedesktop.locale1
CapabilityBoundingSet=

"
(* Test: Systemd.lns *)
test Systemd.lns get unit =
  { "#comment" = "This file is part of systemd." }
  {}
  {  }
  { "#comment" = "See systemd.special(7) for details" }
  {  }
  { ".include" = "/etc/example" }
  {  }
  { "Unit"
    { "Description"
      { "value" = "Locale Service" }
    }
    { "#comment" = "Add another file" }
    { ".include" = "/etc/example" }
    {  }
  }
  { "Service"
    { "ExecStart"
      { "command" = "/lib/systemd/systemd-localed" }
    }
    { "Type"
      { "value" = "dbus" }
    }
    { "BusName"
      { "value" = "org.freedesktop.locale1" }
    }
    { "CapabilityBoundingSet" }
    {  }
  }

(* Test: Systemd.lns
     Values can contain backslashes *)
test Systemd.entry_command get "ExecStart=/usr/bin/find /var/lib/sudo -exec /usr/bin/touch -t 198501010000 '{}' \073\n" =
  { "ExecStart"
    { "command" = "/usr/bin/find" }
    { "arguments"
      { "1" = "/var/lib/sudo" }
      { "2" = "-exec" }
      { "3" = "/usr/bin/touch" }
      { "4" = "-t" }
      { "5" = "198501010000" }
      { "6" = "'{}'" }
      { "7" = "\073" }
    }
  }

