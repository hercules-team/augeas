(* Module Jaas *)
(* Author: Simon Vocella <voxsim@gmail.com> *)
module Test_jaas =

let conf = "
/*
  This is the JAAS configuration file used by the Shibboleth IdP.

  A JAAS configuration file is a grouping of LoginModules defined in the following manner:
  <LoginModuleClass> <Flag> <ModuleOptions>;

  LoginModuleClass - fully qualified class name of the LoginModule class
  Flag             - indicates whether the requirement level for the modules;
                         allowed values: required, requisite, sufficient, optional
  ModuleOptions    - a space delimited list of name=\"value\" options

  For complete documentation on the format of this file see:
  http://java.sun.com/j2se/1.5.0/docs/api/javax/security/auth/login/Configuration.html

  For LoginModules available within the Sun JVM see:
  http://java.sun.com/j2se/1.5.0/docs/guide/security/jaas/tutorials/LoginConfigFile.html

  Warning: Do NOT use Sun's JNDI LoginModule to authentication against an LDAP directory,
  Use the LdapLoginModule that ships with Shibboleth and is demonstrated below.

  Note, the application identifier MUST be ShibUserPassAuth
*/


ShibUserPassAuth {

// Example LDAP authentication
// See: https://wiki.shibboleth.net/confluence/display/SHIB2/IdPAuthUserPass
/*
   edu.vt.middleware.ldap.jaas.LdapLoginModule required
      ldapUrl=\"ldap://ldap.example.org\"
      baseDn=\"ou=people,dc=example,dc=org\"
      ssl=\"true\"
      userFilter=\"uid={0}\";
*/

// Example Kerberos authentication, requires Sun's JVM
// See: https://wiki.shibboleth.net/confluence/display/SHIB2/IdPAuthUserPass
/*
   com.sun.security.auth.module.Krb5LoginModule required
      useKeyTab=\"true\"
      keyTab=\"/path/to/idp/keytab/file\";
*/

   edu.vt.middleware.ldap.jaas.LdapLoginModule required
      host = \"ldap://127.0.0.1:389\"
      base = \"dc=example,dc=com\"
      serviceUser = \"cn=admin,dc=example,dc=com\"
      serviceCredential = \"ldappassword\"
      ssl = \"false\"
      userField = \"uid\"
      // Example comment within definition
      subtreeSearch = \"true\";
};

NetAccountAuth {
   // Test of optionless flag
   nz.ac.auckland.jaas.Krb5LoginModule required;
};

com.sun.security.jgss.krb5.initiate {
   // Test of omitted linebreaks and naked boolean
   com.sun.security.auth.module.Krb5LoginModule required useTicketCache=true;
};"

test Jaas.lns get conf =
  {  }
  { "#mcomment"
    { "1" = "This is the JAAS configuration file used by the Shibboleth IdP." }
    { "2" = "A JAAS configuration file is a grouping of LoginModules defined in the following manner:" }
    { "3" = "<LoginModuleClass> <Flag> <ModuleOptions>;" }
    { "4" = "LoginModuleClass - fully qualified class name of the LoginModule class" }
    { "5" = "Flag             - indicates whether the requirement level for the modules;" }
    { "6" = "allowed values: required, requisite, sufficient, optional" }
    { "7" = "ModuleOptions    - a space delimited list of name=\"value\" options" }
    { "8" = "For complete documentation on the format of this file see:" }
    { "9" = "http://java.sun.com/j2se/1.5.0/docs/api/javax/security/auth/login/Configuration.html" }
    { "10" = "For LoginModules available within the Sun JVM see:" }
    { "11" = "http://java.sun.com/j2se/1.5.0/docs/guide/security/jaas/tutorials/LoginConfigFile.html" }
    { "12" = "Warning: Do NOT use Sun's JNDI LoginModule to authentication against an LDAP directory," }
    { "13" = "Use the LdapLoginModule that ships with Shibboleth and is demonstrated below." }
    { "14" = "Note, the application identifier MUST be ShibUserPassAuth" }
  }
  {  }
  {  }
  { "login" = "ShibUserPassAuth"
    {  }
    { "#comment" = "Example LDAP authentication" }
    { "#comment" = "See: https://wiki.shibboleth.net/confluence/display/SHIB2/IdPAuthUserPass" }
    { "#mcomment"
      { "1" = "edu.vt.middleware.ldap.jaas.LdapLoginModule required" }
      { "2" = "ldapUrl=\"ldap://ldap.example.org\"" }
      { "3" = "baseDn=\"ou=people,dc=example,dc=org\"" }
      { "4" = "ssl=\"true\"" }
      { "5" = "userFilter=\"uid={0}\";" }
    }
    {  }
    { "#comment" = "Example Kerberos authentication, requires Sun's JVM" }
    { "#comment" = "See: https://wiki.shibboleth.net/confluence/display/SHIB2/IdPAuthUserPass" }
    { "#mcomment"
      { "1" = "com.sun.security.auth.module.Krb5LoginModule required" }
      { "2" = "useKeyTab=\"true\"" }
      { "3" = "keyTab=\"/path/to/idp/keytab/file\";" }
    }
    {  }
    { "loginModuleClass" = "edu.vt.middleware.ldap.jaas.LdapLoginModule"
      { "flag" = "required"
        { "host" = "\"ldap://127.0.0.1:389\"" }
        { "base" = "\"dc=example,dc=com\"" }
        { "serviceUser" = "\"cn=admin,dc=example,dc=com\"" }
        { "serviceCredential" = "\"ldappassword\"" }
        { "ssl" = "\"false\"" }
        { "userField" = "\"uid\"" }
        { "#comment" = "Example comment within definition" }
        { "subtreeSearch" = "\"true\"" }
      }
    }
    {  }
  }
  {  }
  {  }
  { "login" = "NetAccountAuth"
    { "#comment" = "Test of optionless flag" }
    { "loginModuleClass" = "nz.ac.auckland.jaas.Krb5LoginModule"
      { "flag" = "required" }
    }
    {  }
  }
  {  }
  {  }
  { "login" = "com.sun.security.jgss.krb5.initiate"
    { "#comment" = "Test of omitted linebreaks and naked boolean" }
    { "loginModuleClass" = "com.sun.security.auth.module.Krb5LoginModule"
      { "flag" = "required"
        { "useTicketCache" = "true" }
      }
    }
    {  }
  }
