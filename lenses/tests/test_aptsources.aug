module Test_aptsources =

  let simple_source = "deb ftp://mirror.bytemark.co.uk/debian/ etch main\n"
  let multi_components = "deb http://security.debian.org/ etch/updates main contrib non-free\n"

  test Aptsources.lns get simple_source =
    { "1"
      { "type"         = "deb" }
      { "uri"          = "ftp://mirror.bytemark.co.uk/debian/" }
      { "distribution" = "etch" }
      { "component"    = "main" }
    }

  test Aptsources.lns get multi_components =
    { "1"
      { "type"         = "deb" }
      { "uri"          = "http://security.debian.org/" }
      { "distribution" = "etch/updates" }
      { "component" = "main" }
      { "component" = "contrib" }
      { "component" = "non-free" }
    }


let multi_line = "#deb http://www.backports.org/debian/ sarge postfix
 # deb http://people.debian.org/~adconrad sarge subversion

deb ftp://mirror.bytemark.co.uk/debian/ etch main non-free contrib
  deb http://security.debian.org/ etch/updates main contrib non-free # security line
	deb-src http://mirror.bytemark.co.uk/debian etch main contrib non-free\n"

  test Aptsources.lns get multi_line =
    { "#comment" = "deb http://www.backports.org/debian/ sarge postfix" } 
    { "#comment" = "deb http://people.debian.org/~adconrad sarge subversion" }
    {}
    { "1"
      { "type"         = "deb" }
      { "uri"          = "ftp://mirror.bytemark.co.uk/debian/" }
      { "distribution" = "etch" }
      { "component" = "main" }
      { "component"   = "non-free" }
      { "component"   = "contrib" }
    }
    { "2"
      { "type"         = "deb" }
      { "uri"          = "http://security.debian.org/" }
      { "distribution" = "etch/updates" }
      { "component"    = "main" }
      { "component"    = "contrib" }
      { "component"    = "non-free" }
    }
    { "3"
      { "type"         = "deb-src" }
      { "uri"          = "http://mirror.bytemark.co.uk/debian" }
      { "distribution" = "etch" }
      { "component"    = "main" }
      { "component"    = "contrib" }
      { "component"    = "non-free" }
    }

    let trailing_comment = "deb ftp://server/debian/ etch main # comment\n"

    (* Should be a noop; makes sure that we preserve the trailing comment *)
    test Aptsources.lns put trailing_comment after
      set "/1/type" "deb"
    = trailing_comment

    (* Support options, GH #295 *)
    test Aptsources.lns get "deb [arch=amd64] tor+http://ftp.us.debian.org/debian sid main contrib
deb [ arch+=amd64 trusted-=true ] http://ftp.us.debian.org/debian sid main contrib\n" =
  { "1"
    { "type" = "deb" }
    { "options"
      { "arch" = "amd64" }
    }
    { "uri" = "tor+http://ftp.us.debian.org/debian" }
    { "distribution" = "sid" }
    { "component" = "main" }
    { "component" = "contrib" } }
  { "2"
    { "type" = "deb" }
    { "options"
      { "arch" = "amd64" { "operation" = "+" } }
      { "trusted" = "true" { "operation" = "-" } }
    }
    { "uri" = "http://ftp.us.debian.org/debian" }
    { "distribution" = "sid" }
    { "component" = "main" }
    { "component" = "contrib" } }

    (* cdrom entries may have spaces, GH #296 *)
    test Aptsources.lns get "deb cdrom:[Debian GNU/Linux 7.5.0 _Wheezy_ - Official amd64 CD Binary-1 20140426-13:37]/ wheezy main\n" =
  { "1"
    { "type" = "deb" }
    { "uri" = "cdrom:[Debian GNU/Linux 7.5.0 _Wheezy_ - Official amd64 CD Binary-1 20140426-13:37]/" }
    { "distribution" = "wheezy" }
    { "component" = "main" } }


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
