module Test_httpd =

(* Check that we can iterate on directive *)
let _ = Httpd.directive+

(* Check that we can do a non iterative section *)
let _ = Httpd.section Httpd.directive

(* directives testing *)
let d1 = "ServerRoot \"/etc/apache2\"\n"
test Httpd.directive get d1 =
  { "directive" = "ServerRoot"
    { "arg" = "\"/etc/apache2\"" }
  }

(* simple quotes *)
let d1s = "ServerRoot '/etc/apache2'\n"
test Httpd.directive get d1s =
  { "directive" = "ServerRoot"
    { "arg" = "'/etc/apache2'" }
  }

let d2 = "ScriptAlias /cgi-bin/ /usr/lib/cgi-bin/\n"
test Httpd.directive get d2 =
  { "directive" = "ScriptAlias"
    { "arg" = "/cgi-bin/" }
    { "arg" = "/usr/lib/cgi-bin/" }
  }

let d3 = "LockFile /var/lock/apache2/accept.lock\n"
test Httpd.directive get d3 =
  { "directive" = "LockFile"
    { "arg" = "/var/lock/apache2/accept.lock" }
  }

let c1 = "
<IfModule>
</IfModule>
"
let c1_put =
"
<IfModule foo bar>
</IfModule>
"


test Httpd.lns get c1 = { }{ "IfModule" }

test Httpd.lns put c1 after set "/IfModule/arg[1]" "foo";
                            set "/IfModule/arg[2]" "bar" = c1_put

let c2 = "
<IfModule !mpm_winnt.c>
  <IfModule !mpm_netware.c>
    LockFile /var/lock/apache2/accept.lock
  </IfModule>
</IfModule>
"

test Httpd.lns get c2 =
  {  }
  { "IfModule"
    { "arg" = "!mpm_winnt.c" }
    { "IfModule"
      { "arg" = "!mpm_netware.c" }
      { "directive" = "LockFile"
        { "arg" = "/var/lock/apache2/accept.lock" }
      }
    }
  }

(* arguments must be the first child of the section *)
test Httpd.lns put c2 after rm "/IfModule/arg";
                            insb "arg" "/IfModule/*[1]";
                            set "/IfModule/arg" "foo"  =
"
<IfModule foo>
  <IfModule !mpm_netware.c>
    LockFile /var/lock/apache2/accept.lock
  </IfModule>
</IfModule>
"

let c3 = "
<IfModule mpm_event_module>
    StartServers          2
    MaxClients          150
    MinSpareThreads      25
    MaxSpareThreads      75
    ThreadLimit          64
    ThreadsPerChild      25
    MaxRequestsPerChild   0
</IfModule>
"

test Httpd.lns get c3 =
  {  }
  { "IfModule"
    { "arg" = "mpm_event_module" }
    { "directive" = "StartServers"
      { "arg" = "2" }
    }
    { "directive" = "MaxClients"
      { "arg" = "150" }
    }
    { "directive" = "MinSpareThreads"
      { "arg" = "25" }
    }
    { "directive" = "MaxSpareThreads"
      { "arg" = "75" }
    }
    { "directive" = "ThreadLimit"
      { "arg" = "64" }
    }
    { "directive" = "ThreadsPerChild"
      { "arg" = "25" }
    }
    { "directive" = "MaxRequestsPerChild"
      { "arg" = "0" }
    }
  }



let c4 = "
<Files ~ \"^\.ht\">
    Order allow,deny
    Deny from all
    Satisfy all
</Files>
"

test Httpd.lns get c4 =
  {  }
  { "Files"
    { "arg" = "~" }
    { "arg" = "\"^\.ht\"" }
    { "directive" = "Order"
      { "arg" = "allow,deny" }
    }
    { "directive" = "Deny"
      { "arg" = "from" }
      { "arg" = "all" }
    }
    { "directive" = "Satisfy"
      { "arg" = "all" }
    }
  }



let c5 = "LogFormat \"%{User-agent}i\" agent\n"
test Httpd.lns get c5 =
  { "directive" = "LogFormat"
    { "arg" = "\"%{User-agent}i\"" }
    { "arg" = "agent" }
  }

let c7 = "LogFormat \"%v:%p %h %l %u %t \\"%r\\" %>s %O \\"%{Referer}i\\" \\"%{User-Agent}i\\"\" vhost_combined\n"
test Httpd.lns get c7 =
  { "directive" = "LogFormat"
    { "arg" = "\"%v:%p %h %l %u %t \\"%r\\" %>s %O \\"%{Referer}i\\" \\"%{User-Agent}i\\"\"" }
    { "arg" = "vhost_combined" }
  }

let c8 = "IndexIgnore .??* *~ *# RCS CVS *,v *,t \n"
test Httpd.directive get c8 =
  { "directive" = "IndexIgnore"
    { "arg" = ".??*" }
    { "arg" = "*~" }
    { "arg" = "*#" }
    { "arg" = "RCS" }
    { "arg" = "CVS" }
    { "arg" = "*,v" }
    { "arg" = "*,t" }
  }

(* FIXME: not yet supported:
 * The backslash "\" may be used as the last character on a line to indicate
 * that the directive continues onto the next line. There must be no other
 * characters or white space between the backslash and the end of the line.
 *)
let multiline = "Options Indexes \
FollowSymLinks MultiViews
"

test Httpd.directive get multiline =
  { "directive" = "Options"
    { "arg" = "Indexes" }
    { "arg" = "FollowSymLinks" }
    { "arg" = "MultiViews" }
  }


let conf2 = "<VirtualHost *:80>
    ServerAdmin webmaster@localhost

    DocumentRoot /var/www
    <Directory />
        Options FollowSymLinks
        AllowOverride None
    </Directory>
    <Directory /var/www/>
        Options Indexes FollowSymLinks MultiViews
        AllowOverride None
        Order allow,deny
        allow from all
    </Directory>

    ScriptAlias /cgi-bin/ /usr/lib/cgi-bin/
    <Directory \"/usr/lib/cgi-bin\">
        AllowOverride None
        Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
        Order allow,deny
        Allow from all
    </Directory>

    ErrorLog /var/log/apache2/error.log

    # Possible values include: debug, info, notice, warn, error, crit,
    # alert, emerg.
    LogLevel warn

    CustomLog /var/log/apache2/access.log combined

    SSLRequireSSL

    Alias /doc/ \"/usr/share/doc/\"
    <Directory \"/usr/share/doc/\">
        Options Indexes MultiViews FollowSymLinks
        AllowOverride None
        Order deny,allow
        Deny from all
        Allow from 127.0.0.0/255.0.0.0 ::1/128
    </Directory>

</VirtualHost>
"

test Httpd.lns get conf2 =
   { "VirtualHost"
    { "arg" = "*:80" }
    { "directive" = "ServerAdmin"
      { "arg" = "webmaster@localhost" }
    }
    {  }
    { "directive" = "DocumentRoot"
      { "arg" = "/var/www" }
    }
    { "Directory"
      { "arg" = "/" }
      { "directive" = "Options"
        { "arg" = "FollowSymLinks" }
      }
      { "directive" = "AllowOverride"
        { "arg" = "None" }
      }
    }
    { "Directory"
      { "arg" = "/var/www/" }
      { "directive" = "Options"
        { "arg" = "Indexes" }
        { "arg" = "FollowSymLinks" }
        { "arg" = "MultiViews" }
      }
      { "directive" = "AllowOverride"
        { "arg" = "None" }
      }
      { "directive" = "Order"
        { "arg" = "allow,deny" }
      }
      { "directive" = "allow"
        { "arg" = "from" }
        { "arg" = "all" }
      }
    }
    {  }
    { "directive" = "ScriptAlias"
      { "arg" = "/cgi-bin/" }
      { "arg" = "/usr/lib/cgi-bin/" }
    }
    { "Directory"
      { "arg" = "\"/usr/lib/cgi-bin\"" }
      { "directive" = "AllowOverride"
        { "arg" = "None" }
      }
      { "directive" = "Options"
        { "arg" = "+ExecCGI" }
        { "arg" = "-MultiViews" }
        { "arg" = "+SymLinksIfOwnerMatch" }
      }
      { "directive" = "Order"
        { "arg" = "allow,deny" }
      }
      { "directive" = "Allow"
        { "arg" = "from" }
        { "arg" = "all" }
      }
    }
    {  }
    { "directive" = "ErrorLog"
      { "arg" = "/var/log/apache2/error.log" }
    }
    {  }
    { "#comment" = "Possible values include: debug, info, notice, warn, error, crit," }
    { "#comment" = "alert, emerg." }
    { "directive" = "LogLevel"
      { "arg" = "warn" }
    }
    {  }
    { "directive" = "CustomLog"
      { "arg" = "/var/log/apache2/access.log" }
      { "arg" = "combined" }
    }
    {  }
    { "directive" = "SSLRequireSSL" }
    {  }
    { "directive" = "Alias"
      { "arg" = "/doc/" }
      { "arg" = "\"/usr/share/doc/\"" }
    }
    { "Directory"
      { "arg" = "\"/usr/share/doc/\"" }
      { "directive" = "Options"
        { "arg" = "Indexes" }
        { "arg" = "MultiViews" }
        { "arg" = "FollowSymLinks" }
      }
      { "directive" = "AllowOverride"
        { "arg" = "None" }
      }
      { "directive" = "Order"
        { "arg" = "deny,allow" }
      }
      { "directive" = "Deny"
        { "arg" = "from" }
        { "arg" = "all" }
      }
      { "directive" = "Allow"
        { "arg" = "from" }
        { "arg" = "127.0.0.0/255.0.0.0" }
        { "arg" = "::1/128" }
      }
    }
    {  }
  }

(* Eol comment *)
test Httpd.lns get "<a> # a comment
MyDirective Foo
</a>\n" =
  { "a"
    { "#comment" = "a comment" }
    { "directive" = "MyDirective" { "arg" = "Foo" } } }

test Httpd.lns get "<a>
# a comment
</a>\n" =
  { "a" { "#comment" = "a comment" } }

(* Test: Httpd.lns
     Newlines inside quoted value (GH issue #104) *)
test Httpd.lns get "Single 'Foo\\
bar'
Double \"Foo\\
bar\"\n" =
  { "directive" = "Single"
    { "arg" = "'Foo\\\nbar'" } }
  { "directive" = "Double"
    { "arg" = "\"Foo\\\nbar\"" } }

(* Test: Httpd.lns
     Support >= in tags (GH #154) *)
let versioncheck = "
<IfVersion = 2.1>
<IfModule !proxy_ajp_module>
LoadModule proxy_ajp_module modules/mod_proxy_ajp.so
</IfModule>
</IfVersion>

<IfVersion >= 2.4>
<IfModule !proxy_ajp_module>
LoadModule proxy_ajp_module modules/mod_proxy_ajp.so
</IfModule>
</IfVersion>
"

test Httpd.lns get versioncheck =
  { }
  { "IfVersion"
    { "arg" = "=" }
    { "arg" = "2.1" }
    { "IfModule"
      { "arg" = "!proxy_ajp_module" }
      { "directive" = "LoadModule"
        { "arg" = "proxy_ajp_module" }
        { "arg" = "modules/mod_proxy_ajp.so" }
      }
    }
  }
  {}
  { "IfVersion"
    { "arg" = ">=" }
    { "arg" = "2.4" }
    { "IfModule"
      { "arg" = "!proxy_ajp_module" }
      { "directive" = "LoadModule"
        { "arg" = "proxy_ajp_module" }
        { "arg" = "modules/mod_proxy_ajp.so" }
      }
    }
  }


(* GH #220 *)
let double_comment = "<IfDefine Foo>
##
## Comment
##
</IfDefine>\n"

test Httpd.lns get double_comment =
  { "IfDefine"
    { "arg" = "Foo" }
    { "#comment" = "#" }
    { "#comment" = "# Comment" }
    { "#comment" = "#" }
  }

let single_comment = "<IfDefine Foo>
#
## Comment
##
</IfDefine>\n"

test Httpd.lns get single_comment =
  { "IfDefine"
    { "arg" = "Foo" }
    { "#comment" = "# Comment" }
    { "#comment" = "#" }
  }

let single_empty = "<IfDefine Foo>
#

</IfDefine>\n"
test Httpd.lns get single_empty =
  { "IfDefine"
    { "arg" = "Foo" }
  }

let eol_empty = "<IfDefine Foo> #
</IfDefine>\n"
test Httpd.lns get eol_empty =
  { "IfDefine"
    { "arg" = "Foo" }
  }

(* Issue #140 *)
test Httpd.lns get "<IfModule mod_ssl.c>
    # one comment
    # another comment
</IfModule>\n" =
  { "IfModule"
    { "arg" = "mod_ssl.c" }
    { "#comment" = "one comment" }
    { "#comment" = "another comment" }
  }
