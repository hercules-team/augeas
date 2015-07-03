(*
Module: Test_Rhsm
  Provides unit tests and examples for the <Rhsm> lens.
*)

module Test_rhsm =

  (* Variable: conf
     A full rhsm.conf *)
  let conf = "# Red Hat Subscription Manager Configuration File:

# Unified Entitlement Platform Configuration
[server]
# Server hostname:
hostname = subscription.rhn.redhat.com

# Server prefix:
prefix = /subscription

# Server port:
port = 443

# Set to 1 to disable certificate validation:
insecure = 0

# Set the depth of certs which should be checked
# when validating a certificate
ssl_verify_depth = 3

# an http proxy server to use
proxy_hostname =

# port for http proxy server
proxy_port =

# user name for authenticating to an http proxy, if needed
proxy_user =

# password for basic http proxy auth, if needed
proxy_password =

[rhsm]
# Content base URL:
baseurl= https://cdn.redhat.com

# Server CA certificate location:
ca_cert_dir = /etc/rhsm/ca/

# Default CA cert to use when generating yum repo configs:
repo_ca_cert = %(ca_cert_dir)sredhat-uep.pem

# Where the certificates should be stored
productCertDir = /etc/pki/product
entitlementCertDir = /etc/pki/entitlement
consumerCertDir = /etc/pki/consumer

# Manage generation of yum repositories for subscribed content:
manage_repos = 1

# Refresh repo files with server overrides on every yum command
full_refresh_on_yum = 0

# If set to zero, the client will not report the package profile to
# the subscription management service.
report_package_profile = 1

# The directory to search for subscription manager plugins
pluginDir = /usr/share/rhsm-plugins

# The directory to search for plugin configuration files
pluginConfDir = /etc/rhsm/pluginconf.d

[rhsmcertd]
# Interval to run cert check (in minutes):
certCheckInterval = 240
# Interval to run auto-attach (in minutes):
autoAttachInterval = 1440
"

  test Rhsm.lns get conf =
    { "#comment" = "Red Hat Subscription Manager Configuration File:" }
    {  }
    { "#comment" = "Unified Entitlement Platform Configuration" }
    { "server"
      { "#comment" = "Server hostname:" }
      { "hostname" = "subscription.rhn.redhat.com" }
      {  }
      { "#comment" = "Server prefix:" }
      { "prefix" = "/subscription" }
      {  }
      { "#comment" = "Server port:" }
      { "port" = "443" }
      {  }
      { "#comment" = "Set to 1 to disable certificate validation:" }
      { "insecure" = "0" }
      {  }
      { "#comment" = "Set the depth of certs which should be checked" }
      { "#comment" = "when validating a certificate" }
      { "ssl_verify_depth" = "3" }
      {  }
      { "#comment" = "an http proxy server to use" }
      { "proxy_hostname" }
      {  }
      { "#comment" = "port for http proxy server" }
      { "proxy_port" }
      {  }
      { "#comment" = "user name for authenticating to an http proxy, if needed" }
      { "proxy_user" }
      {  }
      { "#comment" = "password for basic http proxy auth, if needed" }
      { "proxy_password" }
      {  }
    }
    { "rhsm"
      { "#comment" = "Content base URL:" }
      { "baseurl" = "https://cdn.redhat.com" }
      {  }
      { "#comment" = "Server CA certificate location:" }
      { "ca_cert_dir" = "/etc/rhsm/ca/" }
      {  }
      { "#comment" = "Default CA cert to use when generating yum repo configs:" }
      { "repo_ca_cert" = "%(ca_cert_dir)sredhat-uep.pem" }
      {  }
      { "#comment" = "Where the certificates should be stored" }
      { "productCertDir" = "/etc/pki/product" }
      { "entitlementCertDir" = "/etc/pki/entitlement" }
      { "consumerCertDir" = "/etc/pki/consumer" }
      {  }
      { "#comment" = "Manage generation of yum repositories for subscribed content:" }
      { "manage_repos" = "1" }
      {  }
      { "#comment" = "Refresh repo files with server overrides on every yum command" }
      { "full_refresh_on_yum" = "0" }
      {  }
      { "#comment" = "If set to zero, the client will not report the package profile to" }
      { "#comment" = "the subscription management service." }
      { "report_package_profile" = "1" }
      {  }
      { "#comment" = "The directory to search for subscription manager plugins" }
      { "pluginDir" = "/usr/share/rhsm-plugins" }
      {  }
      { "#comment" = "The directory to search for plugin configuration files" }
      { "pluginConfDir" = "/etc/rhsm/pluginconf.d" }
      {  }
    }
    { "rhsmcertd"
      { "#comment" = "Interval to run cert check (in minutes):" }
      { "certCheckInterval" = "240" }
      { "#comment" = "Interval to run auto-attach (in minutes):" }
      { "autoAttachInterval" = "1440" }
    }
