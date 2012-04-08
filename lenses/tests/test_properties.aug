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
"

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
tomcat.application.host=foo.network.com
"
