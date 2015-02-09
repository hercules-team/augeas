module Test_Mailscanner_Rules =
let conf = "# JKF 10/08/2007 Adobe Acrobat nastiness
rename	\.fdf$			Dangerous Adobe Acrobat data-file						Opening this file can cause auto-loading of any file from the internet

# JKF 04/01/2005 More Microsoft security vulnerabilities
deny	\.ico$			Windows icon file security vulnerability					Possible buffer overflow in Windows
allow	\.(jan|feb|mar|apr|may|jun|june|jul|july|aug|sep|sept|oct|nov|dec)\.[a-z0-9]{3}$	-	-
deny+delete	\.cur$			Windows cursor file security vulnerability					Possible buffer overflow in Windows
andrew@baruwa.com,andrew@baruwa.net	\.reg$		Possible Windows registry attack						Windows registry entries are very dangerous in email
andrew@baruwa.com andrew@baruwa.net	\.chm$		Possible compiled Help file-based virus						Compiled help files are very dangerous in email
rename to .ppt	\.pps$		Renamed .pps to .ppt							Renamed .pps to .ppt
"

test Mailscanner_Rules.lns get conf =
    { "#comment" = "JKF 10/08/2007 Adobe Acrobat nastiness" }
    { "1"
        { "action" = "rename" }
        { "regex" = "\.fdf$" }
        { "log-text" = "Dangerous Adobe Acrobat data-file" }
        { "user-report" = "Opening this file can cause auto-loading of any file from the internet" }
    }
    {}
    { "#comment" = "JKF 04/01/2005 More Microsoft security vulnerabilities" }
    { "2"
        { "action" = "deny" }
        { "regex" = "\.ico$" }
        { "log-text" = "Windows icon file security vulnerability" }
        { "user-report" = "Possible buffer overflow in Windows" }
    }
    { "3"
        { "action" = "allow" }
        { "regex" = "\.(jan|feb|mar|apr|may|jun|june|jul|july|aug|sep|sept|oct|nov|dec)\.[a-z0-9]{3}$" }
        { "log-text" = "-" }
        { "user-report" = "-" }
    }
    { "4"
        { "action" = "deny+delete" }
        { "regex" = "\.cur$" }
        { "log-text" = "Windows cursor file security vulnerability" }
        { "user-report" = "Possible buffer overflow in Windows" }
    }
    { "5"
        { "action" = "andrew@baruwa.com,andrew@baruwa.net" }
        { "regex" = "\.reg$" }
        { "log-text" = "Possible Windows registry attack" }
        { "user-report" = "Windows registry entries are very dangerous in email" }
    }
    { "6"
        { "action" = "andrew@baruwa.com andrew@baruwa.net" }
        { "regex" = "\.chm$" }
        { "log-text" = "Possible compiled Help file-based virus" }
        { "user-report" = "Compiled help files are very dangerous in email" }
    }
    { "7"
        { "action" = "rename to .ppt" }
        { "regex" = "\.pps$" }
        { "log-text" = "Renamed .pps to .ppt" }
        { "user-report" = "Renamed .pps to .ppt" }
    }