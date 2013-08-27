(*
Module: Test_Koji
  Provides unit tests and examples for the <Koji> lens.
*)

module Test_koji =

  (* Variable: conf
     A full koji.conf *)
  let conf = "[koji]

;configuration for koji cli tool

;url of XMLRPC server
server = http://localhost/kojihub

;url of web interface
weburl = http://localhost/koji

;url of package download site
pkgurl = http://localhost/packages

;path to the koji top directory
topdir = /mnt/koji

;configuration for SSL athentication

;client certificate
cert = /etc/pki/koji/kojiadm.pem

;certificate of the CA that issued the client certificate
ca = /etc/pki/koji/koji_ca_cert.crt

;certificate of the CA that issued the HTTP server certificate
serverca = /etc/pki/koji/koji_ca_cert.crt
"

  test Koji.lns get conf =
  { "section" = "koji"
    {  }
    { "#comment" = "configuration for koji cli tool" }
    {  }
    { "#comment" = "url of XMLRPC server" }
    { "server" = "http://localhost/kojihub" }
    {  }
    { "#comment" = "url of web interface" }
    { "weburl" = "http://localhost/koji" }
    {  }
    { "#comment" = "url of package download site" }
    { "pkgurl" = "http://localhost/packages" }
    {  }
    { "#comment" = "path to the koji top directory" }
    { "topdir" = "/mnt/koji" }
    {  }
    { "#comment" = "configuration for SSL athentication" }
    {  }
    { "#comment" = "client certificate" }
    { "cert" = "/etc/pki/koji/kojiadm.pem" }
    {  }
    { "#comment" = "certificate of the CA that issued the client certificate" }
    { "ca" = "/etc/pki/koji/koji_ca_cert.crt" }
    {  }
    { "#comment" = "certificate of the CA that issued the HTTP server certificate" }
    { "serverca" = "/etc/pki/koji/koji_ca_cert.crt" }
  }
