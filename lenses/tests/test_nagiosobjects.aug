module Test_NagiosObjects =
    let conf="
#
# Nagios Objects definitions file
#

define host {
    host_name               plonk
    alias                   plonk
    address                 plonk
    use                     generic_template
    contact_groups          Monitoring-Team,admins
}

define service {
    service_description     gen
    use                     generic_template_passive
    host_name               plonk
    check_command           nopassivecheckreceived
    contact_groups          admins
}

; This is a semicolon comment

define service{
    service_description     gen2
    use                     generic_template_passive
    host_name               plonk
    }
"

    test NagiosObjects.lns get conf =
        {}
        {}
        { "#comment" = "Nagios Objects definitions file" }
        {}
        {}
        { "host"
            { "host_name"       = "plonk" }
            { "alias"           = "plonk" }
            { "address"         = "plonk" }
            { "use"             = "generic_template" }
            { "contact_groups"  = "Monitoring-Team,admins" }
        }
        {}
        { "service"
            { "service_description"     = "gen" }
            { "use"                     = "generic_template_passive" }
            { "host_name"               = "plonk" }
            { "check_command"           = "nopassivecheckreceived" }
            { "contact_groups"          = "admins" }
        }
        {}
        { "#comment" = "This is a semicolon comment" }
        {}
        { "service"
            { "service_description"     = "gen2" }
            { "use"                     = "generic_template_passive" }
            { "host_name"               = "plonk" }
        }

