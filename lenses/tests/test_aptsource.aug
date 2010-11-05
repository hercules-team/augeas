module Test_aptsource =

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

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
