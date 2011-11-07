module Test_mdadm_conf =

let conf = "
# Comment
device containers
 # Comment
DEVICE partitions  \ndev
	/dev/hda*  \n  /dev/hdc*
deVI
ARRAY /dev/md0 UUID=c3d3134f-2aa9-4514-9da3-82ccd1cccc7b Name=foo=bar
    supeR-minor=3 devicEs=/dev/hda,/dev/hdb Level=1 num-devices=5 spares=2
    spare-group=bar auTo=yes BITMAP=/path/to/bitmap metadata=frob
    container=/dev/sda member=1
MAIL # Initial comment
    user@example.com # End of line comment
MAILF user@example.com # MAILFROM can only be abbreviated to 5 characters
PROGRA /usr/sbin/handle-mdadm-events
CREA group=system mode=0640 auto=part-8
HOME <system>
AUT +1.x Homehost -all
POL domain=domain1 metadata=imsm path=pci-0000:00:1f.2-scsi-*
           action=spare
PART domain=domain1 metadata=imsm path=pci-0000:04:00.0-scsi-[01]*
           action=include
"

test Mdadm_conf.lns get conf =
    {}
    { "#comment" = "Comment" }
    { "device"
        { "containers" }
    }
    { "#comment" = "Comment" }
    { "device"
        { "partitions" }
    }
    { "device"
        { "name" = "/dev/hda*" }
        { "name" = "/dev/hdc*" }
    }
    { "device" }
    { "array"
        { "devicename" = "/dev/md0" }
        { "uuid" = "c3d3134f-2aa9-4514-9da3-82ccd1cccc7b" }
        { "name" = "foo=bar" }
        { "super-minor" = "3" }
        { "devices" = "/dev/hda,/dev/hdb" }
        { "level" = "1" }
        { "num-devices" = "5" }
        { "spares" = "2" }
        { "spare-group" = "bar" }
        { "auto" = "yes" }
        { "bitmap" = "/path/to/bitmap" }
        { "metadata" = "frob" }
        { "container" = "/dev/sda" }
        { "member" = "1" }
    }
    { "mailaddr"
        { "#comment" = "Initial comment" }
        { "value" = "user@example.com" }
        { "#comment" = "End of line comment" }
    }
    { "mailfrom"
        { "value" = "user@example.com" }
        { "#comment" = "MAILFROM can only be abbreviated to 5 characters" }
    }
    { "program"
        { "value" = "/usr/sbin/handle-mdadm-events" }
    }
    { "create"
        { "group" = "system" }
        { "mode" = "0640" }
        { "auto" = "part-8" }
    }
    { "homehost"
        { "value" = "<system>" }
    }
    { "auto"
        { "+" = "1.x" }
        { "homehost" }
        { "-" = "all" }
    }
    { "policy"
        { "domain" = "domain1" }
        { "metadata" = "imsm" }
        { "path" = "pci-0000:00:1f.2-scsi-*" }
        { "action" = "spare" }
    }
    { "part-policy"
        { "domain" = "domain1" }
        { "metadata" = "imsm" }
        { "path" = "pci-0000:04:00.0-scsi-[01]*" }
        { "action" = "include" }
    }
