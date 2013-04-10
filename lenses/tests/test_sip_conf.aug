module Test_sip_conf =

let conf = "[general]
context=default                 ; Default context for incoming calls
udpbindaddr=0.0.0.0             ; IP address to bind UDP listen socket to (0.0.0.0 binds to all)
; The address family of the bound UDP address is used to determine how Asterisk performs
; DNS lookups. In cases a) and c) above, only A records are considered. In case b), only
; AAAA records are considered. In case d), both A and AAAA records are considered. Note,


[basic-options-title](!,superclass-template);a template for my preferred codecs !@#$%#@$%^^&%%^*&$%
        #comment after the title
        dtmfmode=rfc2833
        context=from-office
        type=friend


[my-codecs](!)                    ; a template for my preferred codecs
        disallow=all
        allow=ilbc
        allow=g729
        allow=gsm
        allow=g723
        allow=ulaw

[2133](natted-phone,my-codecs) ;;;;; some sort of comment
       secret = peekaboo
[2134](natted-phone,ulaw-phone)
       secret = not_very_secret
[2136](public-phone,ulaw-phone)
       secret = not_very_secret_either
"

test Sip_Conf.lns get conf =
  { "title" = "general"
    { "context" = "default"
      { "#comment" = "Default context for incoming calls" }
    }
    { "udpbindaddr" = "0.0.0.0"
      { "#comment" = "IP address to bind UDP listen socket to (0.0.0.0 binds to all)" }
    }
    { "#comment" = "The address family of the bound UDP address is used to determine how Asterisk performs" }
    { "#comment" = "DNS lookups. In cases a) and c) above, only A records are considered. In case b), only" }
    { "#comment" = "AAAA records are considered. In case d), both A and AAAA records are considered. Note," }
    {  }
    {  }
  }
  { "title" = "basic-options-title"
    { "@is_template" }
    { "@use_template" = "superclass-template" }
    { "#title_comment" = ";a template for my preferred codecs !@#$%#@$%^^&%%^*&$%" }
    { "#comment" = "comment after the title" }
    { "dtmfmode" = "rfc2833" }
    { "context" = "from-office" }
    { "type" = "friend" }
    {  }
    {  }
  }
  { "title" = "my-codecs"
    { "@is_template" }
    { "#title_comment" = "                    ; a template for my preferred codecs" }
    { "disallow" = "all" }
    { "allow" = "ilbc" }
    { "allow" = "g729" }
    { "allow" = "gsm" }
    { "allow" = "g723" }
    { "allow" = "ulaw" }
    {  }
  }
  { "title" = "2133"
    { "@use_template" = "natted-phone" }
    { "@use_template" = "my-codecs" }
    { "#title_comment" = " ;;;;; some sort of comment" }
    { "secret" = "peekaboo" }
  }
  { "title" = "2134"
    { "@use_template" = "natted-phone" }
    { "@use_template" = "ulaw-phone" }
    { "secret" = "not_very_secret" }
  }
  { "title" = "2136"
    { "@use_template" = "public-phone" }
    { "@use_template" = "ulaw-phone" }
    { "secret" = "not_very_secret_either" }
  }

  (*********************************************
  * Tests for update, create, delete
  *
  *********************************************)
  
  (*********************************************
  * Test to confirm that we can update the
  * default context
  *
  *********************************************)
  test Sip_Conf.lns put "[general]\ncontext=default\n" after
      set "title[.='general']/context" "updated"
  = "[general]\ncontext=updated
"

  (*********************************************
  * Test to confirm that we can create a
  * new title with a context
  *
  *********************************************)
  test Sip_Conf.lns put "[general]\ncontext=default\n" after
      set "/title[.='newtitle']" "newtitle"; set "/title[.='newtitle']/context" "foobarbaz"
  = "[general]\ncontext=default
[newtitle]
context=foobarbaz
"

