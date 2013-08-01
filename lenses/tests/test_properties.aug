module Test_properties =
    let conf = "
# Test tomcat properties file
#tomcat.commented.value=1
    # config
tomcat.port = 8080
tomcat.application.name=testapp
    tomcat.application.description=my test application
property.with_underscore=works
empty.property=
empty.property.withtrailingspaces=   \n! more comments
key: value
key2:value2
key3 :value3
key4:=value4

long.description=this is a description that happens to span \
	more than one line with a combination of tabs and \
        spaces \  \nor not

# comment break

short.break = a\
 b

=empty_key
 =empty_key

cheeses

spaces only
multi  spaces
  indented spaces

\= =A
space and = equals
space with \
   multiline

escaped\:colon=value
escaped\=equals=value
escaped\ space=value
"

(* Other tests that aren't supported yet
overflow.description=\
  just wanted to indent it
*)

let lns = Properties.lns

test lns get conf =
    { }
    { "#comment" = "Test tomcat properties file" }
    { "#comment" = "tomcat.commented.value=1" }
    { "#comment" = "config" }
    { "tomcat.port" = "8080" }
    { "tomcat.application.name" = "testapp" }
    { "tomcat.application.description" = "my test application" }
    { "property.with_underscore" = "works" }
    { "empty.property" }
    { "empty.property.withtrailingspaces" }
    { "!comment" = "more comments" }
    { "key" = "value" }
    { "key2" = "value2" }
    { "key3" = "value3" }
    { "key4" = "=value4" }
    {}
    { "long.description" = " < multi > "
        { = "this is a description that happens to span " }
        { = "more than one line with a combination of tabs and " }
        { = "spaces " }
        { = "or not" }
    }
    {}
    { "#comment" = "comment break" }
    {}
    { "short.break" = " < multi > "
        { = "a" }
        { = "b" }
    }
    {}
    { = "empty_key" }
    { = "empty_key" }
    {}
    { "cheeses" }
    {}
    { "spaces" = "only" }
    { "multi" = "spaces" }
    { "indented" = "spaces" }
    {}
    { "\\=" = "A" }
    { "space" = "and = equals" }
    { "space" = " < multi > "
        { = "with " }
        { = "multiline" }
    }
    {}
    { "escaped\:colon" = "value" }
    { "escaped\=equals" = "value" }
    { "escaped\ space" = "value" }
test lns put conf after
    set "tomcat.port" "99";
    set "tomcat.application.host" "foo.network.com"
    = "
# Test tomcat properties file
#tomcat.commented.value=1
    # config
tomcat.port = 99
tomcat.application.name=testapp
    tomcat.application.description=my test application
property.with_underscore=works
empty.property=
empty.property.withtrailingspaces=   \n! more comments
key: value
key2:value2
key3 :value3
key4:=value4

long.description=this is a description that happens to span \
	more than one line with a combination of tabs and \
        spaces \  \nor not

# comment break

short.break = a\
 b

=empty_key
 =empty_key

cheeses

spaces only
multi  spaces
  indented spaces

\= =A
space and = equals
space with \
   multiline

escaped\:colon=value
escaped\=equals=value
escaped\ space=value
tomcat.application.host=foo.network.com
"

(* GH issue #19: value on new line *)
test lns get "k=\
b\
c\n" =
    { "k" = " < multi > "
      { } { = "b" } { = "c" } }

test lns get "tomcat.util.scan.DefaultJarScanner.jarsToSkip=\
bootstrap.jar,commons-daemon.jar,tomcat-juli.jar\n" =
    { "tomcat.util.scan.DefaultJarScanner.jarsToSkip" = " < multi > "
      { } { = "bootstrap.jar,commons-daemon.jar,tomcat-juli.jar" } }

