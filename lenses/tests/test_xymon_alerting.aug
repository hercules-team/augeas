(*
Module: Test_Xymon_Alerting
  Provides unit tests and examples for the <Xymon_Alerting> lens.
*)

module Test_Xymon_Alerting =
    let macro_definition = "$NOTIF_LOCAL=SCRIPT /foo/xymonqpage.sh $PAGER SCRIPT /foo/xymonsms.sh $SMS FORMAT=SMS COLOR=!yellow\n"
    test Xymon_Alerting.lns get macro_definition = 
    { "$NOTIF_LOCAL" = "SCRIPT /foo/xymonqpage.sh $PAGER SCRIPT /foo/xymonsms.sh $SMS FORMAT=SMS COLOR=!yellow" }

    let basic_syntax = "HOST=hostname IGNORE\n"
    test Xymon_Alerting.lns get basic_syntax = 
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let two_filters = "HOST=hostname SERVICE=service IGNORE\n"
    test Xymon_Alerting.lns get two_filters =
      { "1"
        { "filters"
          { "HOST"    = "hostname" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let two_recipients = "HOST=hostname IGNORE STOP\n"
    test Xymon_Alerting.lns get two_recipients =
      { "1"
        { "filters"
          { "HOST"    = "hostname" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
          { "STOP"   { "filters" } }
        }
      }

    let two_lines = "HOST=hostname SERVICE=service\n    IGNORE\n"
    test Xymon_Alerting.lns get two_lines =
      { "1"
        { "filters"
          { "HOST"    = "hostname" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let two_lines_for_recipients = "HOST=hostname SERVICE=service\n    IGNORE\nSTOP\n"
    test Xymon_Alerting.lns get two_lines_for_recipients =
      { "1"
        { "filters"
          { "HOST"    = "hostname" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
          { "STOP"   { "filters" } }
        }
      }

    let with_blanks_at_eol = "HOST=hostname SERVICE=service  \n    IGNORE  \n"
    test Xymon_Alerting.lns get with_blanks_at_eol =
      { "1"
        { "filters"
          { "HOST"    = "hostname" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let several_rules = "HOST=hostname SERVICE=service\nIGNORE\nHOST=hostname2 SERVICE=svc\nIGNORE\nSTOP\n"
    test Xymon_Alerting.lns get several_rules =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }
      { "2"
        { "filters"
          { "HOST" = "hostname2" }
          { "SERVICE" = "svc" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
          { "STOP"   { "filters" } }
        }
      }


    let duration = "HOST=hostname DURATION>20 SERVICE=service\nIGNORE\n"
    test Xymon_Alerting.lns get duration =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
          { "DURATION"
            { "operator" = ">" }
            { "value"    = "20" }
          }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let notice = "HOST=hostname NOTICE SERVICE=service\nIGNORE\n"
    test Xymon_Alerting.lns get notice =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
          { "NOTICE" }
          { "SERVICE" = "service" }
        }
        { "recipients"
          { "IGNORE" { "filters" } }
        }
      }

    let mail = "HOST=hostname MAIL astreinteMail\n"
    test Xymon_Alerting.lns get mail =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "MAIL" = "astreinteMail"
            { "filters" }
          }
        }
      }

    let script = "HOST=hostname SCRIPT /foo/email.sh astreinteMail\n"
    test Xymon_Alerting.lns get script =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "SCRIPT"
            { "script"    = "/foo/email.sh" }
            { "recipient" = "astreinteMail" }
            { "filters" }
          }
        }
      }

    let repeat = "HOST=hostname REPEAT=15\n"
    test Xymon_Alerting.lns get repeat =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "REPEAT" = "15" 
            { "filters" }
          }
        }
      }

    let mail_with_filters = "HOST=hostname MAIL astreinteMail EXSERVICE=service\n"
    test Xymon_Alerting.lns get mail_with_filters =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "MAIL" = "astreinteMail"
            { "filters"
              { "EXSERVICE" = "service" }
            }
          }
        }
      }

    let mail_with_several_filters = "HOST=hostname MAIL astreinteMail EXSERVICE=service DURATION>20\n"
    test Xymon_Alerting.lns get mail_with_several_filters =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "MAIL" = "astreinteMail"
            { "filters"
              { "EXSERVICE" = "service" }
              { "DURATION"
                { "operator" = ">" }
                { "value" = "20" }
              }
            }
          }
        }
      }

    let script_with_several_filters = "HOST=hostname SCRIPT /foo/email.sh astreinteMail EXSERVICE=service DURATION>20\n"
    test Xymon_Alerting.lns get script_with_several_filters =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "SCRIPT"
            { "script"    = "/foo/email.sh" }
            { "recipient" = "astreinteMail" }
            { "filters"
              { "EXSERVICE" = "service" }
              { "DURATION"
                { "operator" = ">" }
                { "value" = "20" }
              }
            }
          }
        }
      }

    let repeat_with_several_filters = "HOST=hostname REPEAT=15 EXSERVICE=service DURATION>20\n"
    test Xymon_Alerting.lns get repeat_with_several_filters =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "REPEAT" = "15" 
            { "filters"
              { "EXSERVICE" = "service" }
              { "DURATION"
                { "operator" = ">" }
                { "value" = "20" }
              }
            }
          }
        }
      }

    let recipients_with_several_filters = "HOST=hostname\nREPEAT=15 EXSERVICE=service DURATION>20\nMAIL astreinteMail TIME=weirdtimeformat\n"
    test Xymon_Alerting.lns get recipients_with_several_filters =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "REPEAT" = "15" 
            { "filters"
              { "EXSERVICE" = "service" }
              { "DURATION"
                { "operator" = ">" }
                { "value" = "20" }
              }
            }
          }
          { "MAIL" = "astreinteMail"
            { "filters"
              { "TIME" = "weirdtimeformat" }
            }
          }
        }
      }

    let recipient_macro = "HOST=hostname\n    $NOTIF_LOCAL\n    STOP\n"
    test Xymon_Alerting.lns get recipient_macro =
      { "1"
        { "filters"
          { "HOST" = "hostname" }
        }
        { "recipients"
          { "$NOTIF_LOCAL"
            { "filters" }
          }
          { "STOP"
            { "filters" }
          }
        }
      }

