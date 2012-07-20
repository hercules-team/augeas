module Test_debctrl =

 let source = "Source: libtest-distmanifest-perl\n"
 let source_result =   { "Source" = "libtest-distmanifest-perl" }

 test (Debctrl.simple_entry Debctrl.simple_src_keyword ) get source =
    source_result

 test (Debctrl.simple_entry Debctrl.simple_src_keyword ) get 
  "Maintainer: Debian Perl Group <pkg-perl-maintainers@lists.alioth.debian.org>\n"
   = {  "Maintainer" = "Debian Perl Group <pkg-perl-maintainers@lists.alioth.debian.org>"
     }

 let uploaders 
   = "Uploaders: foo@bar, Dominique Dumont <dominique.dumont@xx.yyy>,\n"
   . "  gregor herrmann <gregoa@xxx.yy>\n"

 let uploaders_result =
    { "Uploaders"
       { "1" = "foo@bar"}
       { "2" = "Dominique Dumont <dominique.dumont@xx.yyy>" }
       { "3" = "gregor herrmann <gregoa@xxx.yy>" } }

 test Debctrl.uploaders get uploaders = uploaders_result

(* test package dependencies *)
test Debctrl.version_depends get "( >= 5.8.8-12 )" = 
   { "version" { "relation"  = ">=" } { "number"  = "5.8.8-12" } }

test Debctrl.arch_depends get "[ !hurd-i386]" = 
   { "arch" { "prefix"  = "!" } { "name"  = "hurd-i386" } }

test Debctrl.arch_depends get "[ hurd-i386]" = 
   { "arch" { "prefix"  = "" } { "name"  = "hurd-i386" } }

let p_depends_test = "perl ( >= 5.8.8-12 ) [ !hurd-i386]"

test Debctrl.package_depends get p_depends_test =
   { "perl"
       { "version"
                   { "relation"  = ">=" }
                   { "number"  = "5.8.8-12" } }
       { "arch" { "prefix"  = "!" } { "name"  = "hurd-i386" } } }

let dependency_test = "perl-modules (>= 5.10) | libmodule-build-perl"

test Debctrl.dependency get dependency_test = 
   { "or" { "perl-modules" 
                { "version" { "relation"  = ">=" } 
                            { "number"  = "5.10" } } } }
   { "or" { "libmodule-build-perl" } }

test (Debctrl.dependency_list "Build-Depends-Indep") get 
  "Build-Depends-Indep: perl (>= 5.8.8-12) [ !hurd-i386], \n"
  . "   perl-modules (>= 5.10) | libmodule-build-perl,\n"
  . "   libcarp-assert-more-perl,\n"
  . "   libconfig-tiny-perl\n"
  = { "Build-Depends-Indep"
       { "and" { "or" { "perl" 
                        { "version"
                          { "relation"  = ">=" }
                          { "number"  = "5.8.8-12" } }
                        { "arch" 
                          { "prefix"  = "!" } 
                          { "name"  = "hurd-i386" } } } } }
       { "and" { "or" { "perl-modules" 
                        { "version" { "relation"  = ">=" }  
                                    { "number"  = "5.10" } } } }
               { "or" { "libmodule-build-perl" } } }
       { "and" { "or" { "libcarp-assert-more-perl" } } }
       { "and" { "or" { "libconfig-tiny-perl" } } } }

test (Debctrl.dependency_list "Depends") get 
  "Depends: ${perl:Depends}, ${misc:Depends},\n"
  ." libparse-recdescent-perl (>= 1.90.0)\n"
  = { "Depends"
       {  "and" { "or" { "${perl:Depends}" }} }
       {  "and" { "or" { "${misc:Depends}" }} }
       {  "and" { "or" { "libparse-recdescent-perl"  
                         { "version"
                           { "relation"  = ">=" }
                           { "number"  = "1.90.0" } } } } }
    } 

 let description = "Description: describe and edit configuration data\n"
 ." Config::Model enables [...] must:\n"
 ."    - if the configuration data\n"
 ." .\n"
 ." With the elements above, (...) on ReadLine.\n"

 test Debctrl.description get description = 
  { "Description" 
    { "summary" = "describe and edit configuration data" }
    { "text" = "Config::Model enables [...] must:" }
    { "text" = "   - if the configuration data" }
    { "text" = "." }            
    { "text" = "With the elements above, (...) on ReadLine."} }
 

 let simple_bin_pkg1 = "Package: libconfig-model-perl\n"
     . "Architecture: all\n"
     . "Description: dummy1\n"
     . " dummy text 1\n"

 let simple_bin_pkg2 = "Package: libconfig-model2-perl\n"
     . "Architecture: all\n"
     . "Description: dummy2\n"
     . " dummy text 2\n"

 test Debctrl.src_entries get source.uploaders 
 =  { "Source" = "libtest-distmanifest-perl" }
                { "Uploaders"
                  { "1" = "foo@bar"}
                  { "2" = "Dominique Dumont <dominique.dumont@xx.yyy>" }
                  { "3" = "gregor herrmann <gregoa@xxx.yy>" } }

 test Debctrl.bin_entries get simple_bin_pkg1 = 
  { "Package" = "libconfig-model-perl" }
  { "Architecture" = "all" } 
  { "Description" { "summary" = "dummy1" } {"text" = "dummy text 1" } }

 
 let paragraph_simple = source . uploaders ."\n" 
       . simple_bin_pkg1 . "\n" 
       . simple_bin_pkg2 

 test Debctrl.lns get paragraph_simple =
   { "srcpkg"   { "Source" = "libtest-distmanifest-perl" }
                { "Uploaders"
                  { "1" = "foo@bar"}
                  { "2" = "Dominique Dumont <dominique.dumont@xx.yyy>" }
                  { "3" = "gregor herrmann <gregoa@xxx.yy>" } } }
   { "binpkg" { "Package" = "libconfig-model-perl" }
                    { "Architecture" = "all" } 
                    { "Description" { "summary" = "dummy1" } 
                                    { "text" = "dummy text 1" } } }
   { "binpkg" { "Package" = "libconfig-model2-perl" }
                    { "Architecture" = "all" } 
		    { "Description" { "summary" = "dummy2" } 
                                    { "text" = "dummy text 2" } } } 


(* PUT TESTS *)

test Debctrl.src_entries
     put uploaders  
     after set "/Uploaders/1" "foo@bar"
   =  uploaders

test Debctrl.src_entries
     put uploaders  
  after set "/Uploaders/1" "bar@bar" 
 =  "Uploaders: bar@bar, Dominique Dumont <dominique.dumont@xx.yyy>,\n"
   . "  gregor herrmann <gregoa@xxx.yy>\n"

test Debctrl.src_entries
     put uploaders  
     after set "/Uploaders/4" "baz@bar"
   =  "Uploaders: foo@bar, Dominique Dumont <dominique.dumont@xx.yyy>,\n"
   . "  gregor herrmann <gregoa@xxx.yy>,\n"
   . " baz@bar\n"

test Debctrl.lns put (source."\nPackage: test\nDescription: foobar\n")
  after
  set "/srcpkg/Uploaders/1" "foo@bar" ;
  set "/srcpkg/Uploaders/2" "Dominique Dumont <dominique.dumont@xx.yyy>" ;
  set "/srcpkg/Uploaders/3" "gregor herrmann <gregoa@xxx.yy>" ;
  set "/srcpkg/Build-Depends-Indep/and[1]/or/perl/version/relation" ">=" ;
  set "/srcpkg/Build-Depends-Indep/and[1]/or/perl/version/number" "5.8.8-12" ;
  set "/srcpkg/Build-Depends-Indep/and[1]/or/perl/arch/prefix" "!" ;
  set "/srcpkg/Build-Depends-Indep/and[1]/or/perl/arch/name" "hurd-i386" ;
  set "/srcpkg/Build-Depends-Indep/and[2]/or[1]/perl-modules/version/relation" ">=" ;
  set "/srcpkg/Build-Depends-Indep/and[2]/or[1]/perl-modules/version/number" "5.10" ;
  set "/srcpkg/Build-Depends-Indep/and[2]/or[2]/libmodule-build-perl" "";
  set "/srcpkg/Build-Depends-Indep/and[3]/or/libcarp-assert-more-perl" "" ;
  set "/srcpkg/Build-Depends-Indep/and[4]/or/libconfig-tiny-perl" "" ;
  set "/binpkg[1]/Package" "libconfig-model-perl"  ; 
  (* must remove description because set cannot insert Archi before description *)
  rm  "/binpkg[1]/Description" ;
  set "/binpkg/Architecture" "all"  ;
  set "/binpkg[1]/Description/summary" "dummy1" ;
  set "/binpkg[1]/Description/text" "dummy text 1" ;
  set "/binpkg[2]/Package" "libconfig-model2-perl" ;
  set "/binpkg[2]/Architecture" "all" ;
  set "/binpkg[2]/Description/summary" "dummy2" ;
  set "/binpkg[2]/Description/text" "dummy text 2" 
  =  
"Source: libtest-distmanifest-perl
Uploaders: foo@bar,
 Dominique Dumont <dominique.dumont@xx.yyy>,
 gregor herrmann <gregoa@xxx.yy>
Build-Depends-Indep: perl ( >= 5.8.8-12 ) [ !hurd-i386 ],
 perl-modules ( >= 5.10 ) | libmodule-build-perl,
 libcarp-assert-more-perl,
 libconfig-tiny-perl

Package: libconfig-model-perl
Architecture: all
Description: dummy1
 dummy text 1

Package: libconfig-model2-perl
Architecture: all
Description: dummy2
 dummy text 2
"

(* Test Augeas' own control file *)
let augeas_control = "Source: augeas
Priority: optional
Maintainer: Nicolas Valcárcel Scerpella (Canonical) <nicolas.valcarcel@canonical.com>
Uploaders: Free Ekanayaka <freee@debian.org>, Micah Anderson <micah@debian.org>
Build-Depends: debhelper (>= 5), autotools-dev, libreadline-dev, chrpath,
 naturaldocs (>= 1.51-1), texlive-latex-base
Standards-Version: 3.9.2
Section: libs
Homepage: http://augeas.net/
DM-Upload-Allowed: yes

Package: augeas-tools
Section: admin
Architecture: any
Depends: ${shlibs:Depends}, ${misc:Depends}
Description: Augeas command line tools
 Augeas is a configuration editing tool. It parses configuration files in their
 native formats and transforms them into a tree. Configuration changes are made
 by manipulating this tree and saving it back into native config files.
 .
 This package provides command line tools based on libaugeas0:
 - augtool, a tool to manage configuration files.
 - augparse, a testing and debugging tool for augeas lenses.

Package: libaugeas-dev
Section: libdevel
Architecture: any
Depends: libaugeas0 (= ${binary:Version}), ${shlibs:Depends}, ${misc:Depends}
Description: Development files for writing applications based on libaugeas0
 Augeas is a configuration editing tool. It parses configuration files in their
 native formats and transforms them into a tree. Configuration changes are made
 by manipulating this tree and saving it back into native config files.
 .
 This package includes the development files to write programs using the Augeas
 API.
"
test DebCtrl.lns get augeas_control =
  { "srcpkg"
    { "Source" = "augeas" }
    { "Priority" = "optional" }
    { "Maintainer" = "Nicolas Valcárcel Scerpella (Canonical) <nicolas.valcarcel@canonical.com>" }
    { "Uploaders"
      { "1" = "Free Ekanayaka <freee@debian.org>" }
      { "2" = "Micah Anderson <micah@debian.org>" }
    }
    { "Build-Depends"
      { "and"
        { "or"
          { "debhelper"
            { "version"
              { "relation" = ">=" }
              { "number" = "5" }
            }
          }
        }
      }
      { "and"
        { "or"
          { "autotools-dev" }
        }
      }
      { "and"
        { "or"
          { "libreadline-dev" }
        }
      }
      { "and"
        { "or"
          { "chrpath" }
        }
      }
      { "and"
        { "or"
          { "naturaldocs"
            { "version"
              { "relation" = ">=" }
              { "number" = "1.51-1" }
            }
          }
        }
      }
      { "and"
        { "or"
          { "texlive-latex-base" }
        }
      }
    }
    { "Standards-Version" = "3.9.2" }
    { "Section" = "libs" }
    { "Homepage" = "http://augeas.net/" }
    { "DM-Upload-Allowed" = "yes" }
  }
  { "binpkg"
    { "Package" = "augeas-tools" }
    { "Section" = "admin" }
    { "Architecture" = "any" }
    { "Depends"
      { "and"
        { "or"
          { "${shlibs:Depends}" }
        }
      }
      { "and"
        { "or"
          { "${misc:Depends}" }
        }
      }
    }
    { "Description"
      { "summary" = "Augeas command line tools" }
      { "text" = "Augeas is a configuration editing tool. It parses configuration files in their" }
      { "text" = "native formats and transforms them into a tree. Configuration changes are made" }
      { "text" = "by manipulating this tree and saving it back into native config files." }
      { "text" = "." }
      { "text" = "This package provides command line tools based on libaugeas0:" }
      { "text" = "- augtool, a tool to manage configuration files." }
      { "text" = "- augparse, a testing and debugging tool for augeas lenses." }
    }
  }
  { "binpkg"
    { "Package" = "libaugeas-dev" }
    { "Section" = "libdevel" }
    { "Architecture" = "any" }
    { "Depends"
      { "and"
        { "or"
          { "libaugeas0"
            { "version"
              { "relation" = "=" }
              { "number" = "${binary:Version}" }
            }
          }
        }
      }
      { "and"
        { "or"
          { "${shlibs:Depends}" }
        }
      }
      { "and"
        { "or"
          { "${misc:Depends}" }
        }
      }
    }
    { "Description"
      { "summary" = "Development files for writing applications based on libaugeas0" }
      { "text" = "Augeas is a configuration editing tool. It parses configuration files in their" }
      { "text" = "native formats and transforms them into a tree. Configuration changes are made" }
      { "text" = "by manipulating this tree and saving it back into native config files." }
      { "text" = "." }
      { "text" = "This package includes the development files to write programs using the Augeas" }
      { "text" = "API." }
    }
  }

(* Bug #267: Python module extensions, from Debian Python Policy, chapter 2 *)
let python_control = "Source: graphite-web
Maintainer: Will Pearson (Editure Key) <wpearson@editure.co.uk>
Section: python
Priority: optional
Build-Depends: debhelper (>= 7), python-support (>= 0.8.4)
Standards-Version: 3.7.2
XS-Python-Version: current

Package: python-graphite-web
Architecture: all
Depends: ${python:Depends}
XB-Python-Version: ${python:Versions}
Provides: ${python:Provides}
Description: Enterprise scalable realtime graphing
"
test Debctrl.lns get python_control =
  { "srcpkg"
    { "Source" = "graphite-web" }
    { "Maintainer" = "Will Pearson (Editure Key) <wpearson@editure.co.uk>" }
    { "Section" = "python" }
    { "Priority" = "optional" }
    { "Build-Depends"
      { "and"
        { "or"
          { "debhelper"
            { "version"
              { "relation" = ">=" }
              { "number" = "7" }
            }
          }
        }
      }
      { "and"
        { "or"
          { "python-support"
            { "version"
              { "relation" = ">=" }
              { "number" = "0.8.4" }
            }
          }
        }
      }
    }
    { "Standards-Version" = "3.7.2" }
    { "XS-Python-Version" = "current" }
  }
  { "binpkg"
    { "Package" = "python-graphite-web" }
    { "Architecture" = "all" }
    { "Depends"
      { "and"
        { "or"
          { "${python:Depends}" }
        }
      }
    }
    { "XB-Python-Version" = "${python:Versions}" }
    { "Provides"
      { "and"
        { "or"
          { "${python:Provides}" }
        }
      }
    }
    { "Description"
      { "summary" = "Enterprise scalable realtime graphing" }
    }
  }

