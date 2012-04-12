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
empty.property.withtrailingspaces=   
! more comments
key: value
key2:value2
key3 :value3
key4:=value4

long.description=this is a description that happens to span \
	more than one line with a combination of tabs and \
        spaces \  
or not

# comment break

short.break = a\
 b

=empty_key
 =empty_key

cheeses

escaped\:colon=value
escaped\=equals=value
"

(* Other tests that aren't supported yet
overflow.description=\
  just wanted to indent it

spaces only
multi  spaces
  indented spaces

escaped\ space=value
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
    { "escaped\:colon" = "value" }
    { "escaped\=equals" = "value" }
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
empty.property.withtrailingspaces=   
! more comments
key: value
key2:value2
key3 :value3
key4:=value4

long.description=this is a description that happens to span \
	more than one line with a combination of tabs and \
        spaces \  
or not

# comment break

short.break = a\
 b

=empty_key
 =empty_key

cheeses

escaped\:colon=value
escaped\=equals=value
tomcat.application.host=foo.network.com
"
