module Test_Pylonspaste =
    let pylons_conf ="# baruwa - Pylons configuration
# The %(here)s variable will be replaced with the parent directory of this file
[uwsgi]
socket = /var/run/baruwa/baruwa.sock
processes = 5
uid = baruwa
daemonize = /var/log/uwsgi/uwsgi-baruwa.log

[server:main]
use = egg:Paste#http
host = 0.0.0.0
port = 5000

[app:main]
use = egg:baruwa
full_stack = true
static_files = false
set debug = false

[identifiers]
plugins =
      form;browser
      auth_tkt

[authenticators]
plugins =
    sa_auth
    baruwa_pop3_auth
    baruwa_imap_auth
    baruwa_smtp_auth
    baruwa_ldap_auth
    baruwa_radius_auth
"

test Pylonspaste.lns get pylons_conf =
    { "#comment" = "baruwa - Pylons configuration" }
    { "#comment" = "The %(here)s variable will be replaced with the parent directory of this file" }
    { "uwsgi"
        { "socket" = "/var/run/baruwa/baruwa.sock" }
        { "processes" = "5" }
        { "uid" = "baruwa" }
        { "daemonize" = "/var/log/uwsgi/uwsgi-baruwa.log" }
        { }
    }
    { "server:main"
        { "use" = "egg:Paste#http" }
        { "host" = "0.0.0.0" }
        { "port" = "5000" }
        { }
    }
    { "app:main"
        { "use" = "egg:baruwa" }
        { "full_stack" = "true" }
        { "static_files" = "false" }
        { "debug" = "false" }
        { }
    }
    { "identifiers"
        { "plugins"
            { "1" = "form;browser" }
            { "2" = "auth_tkt" }
        }
        {}
    }
    { "authenticators"
        { "plugins"
            { "1" = "sa_auth" }
            { "2" = "baruwa_pop3_auth" }
            { "3" = "baruwa_imap_auth" }
            { "4" = "baruwa_smtp_auth" }
            { "5" = "baruwa_ldap_auth" }
            { "6" = "baruwa_radius_auth" }
        }
    }
