module Test_slapd =

let conf = "# This is the main slapd configuration file. See slapd.conf(5) for more
# info on the configuration options.

#######################################################################
# Global Directives:

# Features to permit
#allow bind_v2

# Schema and objectClass definitions
include         /etc/ldap/schema/core.schema

#######################################################################
# Specific Directives for database #1, of type hdb:
# Database specific directives apply to this databasse until another
# 'database' directive occurs
database        hdb

# The base of your directory in database #1
suffix          \"dc=nodomain\"

access to attrs=userPassword,shadowLastChange
        by dn=\"cn=admin,dc=nodomain\" write
        by anonymous auth
        by self write
        by * none
"

test Slapd.lns get conf =
  { "#comment" = "This is the main slapd configuration file. See slapd.conf(5) for more" }
  { "#comment" = "info on the configuration options." }
  {}
  { "#comment" = "######################################################################" }
  { "#comment" = "Global Directives:"}
  {}
  { "#comment" = "Features to permit" }
  { "#comment" = "allow bind_v2" }
  {}
  { "#comment" = "Schema and objectClass definitions" }
  { "include"  = "/etc/ldap/schema/core.schema" }
  {}
  { "#comment" = "######################################################################" }
  { "#comment" = "Specific Directives for database #1, of type hdb:" }
  { "#comment" = "Database specific directives apply to this databasse until another" }
  { "#comment" = "'database' directive occurs" }
  { "database" = "hdb"
     {}
     { "#comment" = "The base of your directory in database #1" }
     { "suffix"   = "dc=nodomain" }
     {}
     { "access to" = "attrs=userPassword,shadowLastChange"
        { "by" = "dn=\"cn=admin,dc=nodomain\""
           { "access" = "write" } }
        { "by" = "anonymous"
           { "access" = "auth" } }
        { "by" = "self"
           { "access" = "write" } }
        { "by" = "*"
           { "access" = "none" } } } }

(* Test: Slapd.lns
     Full access test with who/access/control *)
test Slapd.lns get "access to dn.subtree=\"dc=example,dc=com\"
  by self write stop\n" =
  { "access to" = "dn.subtree=\"dc=example,dc=com\""
    { "by" = "self"
      { "access" = "write" }
      { "control" = "stop" } } }

(* Test: Slapd.lns
     access test with who *)
test Slapd.lns get "access to dn.subtree=\"dc=example,dc=com\"
  by self\n" =
  { "access to" = "dn.subtree=\"dc=example,dc=com\""
    { "by" = "self" } }

(* Test: Slapd.lns
     access test with who/access *)
test Slapd.lns get "access to dn.subtree=\"dc=example,dc=com\"
  by self write\n" =
  { "access to" = "dn.subtree=\"dc=example,dc=com\""
    { "by" = "self"
      { "access" = "write" } } }

(* Test: Slapd.lns
     access test with who/control *)
test Slapd.lns get "access to dn.subtree=\"dc=example,dc=com\"
  by self stop\n" =
  { "access to" = "dn.subtree=\"dc=example,dc=com\""
    { "by" = "self"
      { "control" = "stop" } } }

