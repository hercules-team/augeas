module Test_krb5 =

  (* Krb5.conf from Fermilab *)
  let fermi_str = "###
### This krb5.conf template is intended for use with Fermi
### Kerberos v1_2 and later.  Earlier versions may choke on the
### \"auth_to_local = \" lines unless they are commented out.
### The installation process should do all the right things in
### any case, but if you are reading this and haven't updated
### your kerberos product to v1_2 or later, you really should!
###
[libdefaults]
	ticket_lifetime = 1560m
	default_realm = FNAL.GOV
	ccache_type = 4
	default_tgs_enCtypes = des-cbc-crc
	default_tkt_enctypes = des-cbc-crc
	permitted_enctypes = des-cbc-crc des3-cbc-sha1
	default_lifetime = 7d
	renew_lifetime = 7d
	autologin = true
	forward = true
	forwardable = true
	renewable = true
	encrypt = true
        v4_name_convert = {
                host = {
                        rcmd = host
                        }
                }

[realms]
	FNAL.GOV = {
		kdc = krb-fnal-1.fnal.gov:88
		kdc = krb-fnal-2.fnal.gov:88
		kdc = krb-fnal-3.fnal.gov:88
		kdc = krb-fnal-4.fnal.gov:88
		kdc = krb-fnal-5.fnal.gov:88
		kdc = krb-fnal-6.fnal.gov:88
		kdc = krb-fnal-7.fnal.gov:88
		master_kdc = krb-fnal-admin.fnal.gov:88
		admin_server = krb-fnal-admin.fnal.gov
		default_domain = fnal.gov
	}
	WIN.FNAL.GOV = {
		kdc = littlebird.win.fnal.gov:88
		kdc = bigbird.win.fnal.gov:88
		default_domain = fnal.gov
	}
	FERMI.WIN.FNAL.GOV = {
		kdc = sully.fermi.win.fnal.gov:88
		kdc = elmo.fermi.win.fnal.gov:88
		kdc = grover.fermi.win.fnal.gov:88
		kdc = oscar.fermi.win.fnal.gov:88
		kdc = cookie.fermi.win.fnal.gov:88
		kdc = herry.fermi.win.fnal.gov:88
		default_domain = fnal.gov
	}
	UCHICAGO.EDU = {
		kdc = kerberos-0.uchicago.edu
		kdc = kerberos-1.uchicago.edu
		kdc = kerberos-2.uchicago.edu
		admin_server = kerberos.uchicago.edu
		default_domain = uchicago.edu
	}
	PILOT.FNAL.GOV = {
		kdc = i-krb-2.fnal.gov:88
		master_kdc = i-krb-2.fnal.gov:88
		admin_server = i-krb-2.fnal.gov
		default_domain = fnal.gov
        }
	WINBETA.FNAL.GOV = {
		kdc = wbdc1.winbeta.fnal.gov:88
		kdc = wbdc2.winbeta.fnal.gov:88
		default_domain = fnal.gov
	}
	FERMIBETA.WINBETA.FNAL.GOV = {
		kdc = fbdc1.fermibeta.winbeta.fnal.gov:88
		kdc = fbdc2.fermibeta.winbeta.fnal.gov:88
		default_domain = fnal.gov
	}
	CERN.CH = {
		kdc = afsdb2.cern.ch
		kdc = afsdb3.cern.ch
		kdc = afsdb1.cern.ch
		default_domain = cern.ch
		kpasswd_server = afskrb5m.cern.ch
		admin_server = afskrb5m.cern.ch
		v4_name_convert = {
                        host = {
                                rcmd = host
                        }
                }
	}

[instancemapping]
 afs = {
 	cron/* = \"\"
 	cms/* = \"\"
 	afs/* = \"\"
 	e898/* = \"\"
 }

[capaths]

# FNAL.GOV and PILOT.FNAL.GOV are the MIT Kerberos Domains
# FNAL.GOV is production and PILOT is for testing
# The FERMI Windows domain uses the WIN.FNAL.GOV root realm
# with the FERMI.WIN.FNAL.GOV sub-realm where machines and users
# reside.  The WINBETA and FERMIBETA domains are the equivalent
# testing realms for the FERMIBETA domain.  The 2-way transitive
# trust structure of this complex is as follows:
#
# FNAL.GOV <=> PILOT.FNAL.GOV
# FNAL.GOV <=> WIN.FERMI.GOV <=> FERMI.WIN.FERMI.GOV
# PILOT.FNAL.GOV <=> WINBETA.FNAL.GOV <=> FERMIBETA.WINBETA.FNAL.GOV

FNAL.GOV = {
	PILOT.FNAL.GOV = .
	FERMI.WIN.FNAL.GOV = WIN.FNAL.GOV
	WIN.FNAL.GOV = .
	FERMIBETA.WINBETA.FNAL.GOV = WINBETA.FNAL.GOV
	WINBETA.FNAL.GOV = PILOT.FNAL.GOV
}
PILOT.FNAL.GOV = {
	FNAL.GOV = .
	FERMI.WIN.FNAL.GOV = WIN.FNAL.GOV
	WIN.FNAL.GOV = FNAL.GOV
	FERMIBETA.WINBETA.FNAL.GOV = WINBETA.FNAL.GOV
	WINBETA.FNAL.GOV = .
}
WIN.FNAL.GOV = {
	FNAL.GOV = .
	PILOT.FNAL.GOV = FNAL.GOV
	FERMI.WIN.FNAL.GOV = .
	FERMIBETA.WINBETA.FNAL.GOV = WINBETA.FNAL.GOV
	WINBETA.FNAL.GOV = PILOT.FNAL.GOV
}
WINBETA.FNAL.GOV = {
	PILOT.FNAL.GOV = .
	FERMIBETA.WINBETA.FNAL.GOV = .
	FNAL.GOV = PILOT.FNAL.GOV
	FERMI.WIN.FNAL.GOV = WIN.FNAL.GOV
	WIN.FNAL.GOV = PILOT.FNAL.GOV
}

[logging]
	kdc = SYSLOG:info:local1
	admin_server = SYSLOG:info:local2
	default = SYSLOG:err:auth

[domain_realm]
# Fermilab's (non-windows-centric) domains
	.fnal.gov = FNAL.GOV
	.cdms-soudan.org = FNAL.GOV
	.deemz.net = FNAL.GOV
	.dhcp.fnal.gov = FNAL.GOV
	.minos-soudan.org = FNAL.GOV
	i-krb-2.fnal.gov = PILOT.FNAL.GOV
	.win.fnal.gov = WIN.FNAL.GOV
	.fermi.win.fnal.gov = FERMI.WIN.FNAL.GOV
	.winbeta.fnal.gov = WINBETA.FNAL.GOV
	.fermibeta.winbeta.fnal.gov = FERMIBETA.WINBETA.FNAL.GOV
# Fermilab's KCA servers so FERMI.WIN principals work in FNAL.GOV realm
#	winserver.fnal.gov = FERMI.WIN.FNAL.GOV
#	winserver2.fnal.gov = FERMI.WIN.FNAL.GOVA
# Accelerator nodes to FERMI.WIN for Linux/OS X users
	adgroups.fnal.gov = FERMI.WIN.FNAL.GOV
	adusers.fnal.gov = FERMI.WIN.FNAL.GOV
	webad.fnal.gov = FERMI.WIN.FNAL.GOV
# Friends and family (by request)
	.cs.ttu.edu = FNAL.GOV
	.geol.uniovi.es = FNAL.GOV
	.harvard.edu = FNAL.GOV
	.hpcc.ttu.edu = FNAL.GOV
	.infn.it = FNAL.GOV
	.knu.ac.kr  = FNAL.GOV
	.lns.mit.edu = FNAL.GOV
	.ph.liv.ac.uk = FNAL.GOV
	.pha.jhu.edu = FNAL.GOV
	.phys.ttu.edu = FNAL.GOV
	.phys.ualberta.ca = FNAL.GOV
	.physics.lsa.umich.edu = FNAL.GOV
	.physics.ucla.edu = FNAL.GOV
	.physics.ucsb.edu = FNAL.GOV
	.physics.utoronto.ca = FNAL.GOV
	.rl.ac.uk = FNAL.GOV
	.rockefeller.edu = FNAL.GOV
	.rutgers.edu = FNAL.GOV
	.sdsc.edu = FNAL.GOV
	.sinica.edu.tw = FNAL.GOV
	.tsukuba.jp.hep.net = FNAL.GOV
	.ucsd.edu = FNAL.GOV
	.unl.edu = FNAL.GOV
	.in2p3.fr = FNAL.GOV
	.wisc.edu = FNAL.GOV
	.pic.org.es = FNAL.GOV
	.kisti.re.kr = FNAL.GOV

# The whole \"top half\" is replaced during \"ups installAsRoot krb5conf\", so:
# It would probably be a bad idea to change anything on or above this line

# If you need to add any .domains or hosts, put them here
[domain_realm]
	mojo.lunet.edu = FNAL.GOV

[appdefaults]
	default_lifetime = 7d
	retain_ccache = false
	autologin = true
	forward = true
	forwardable = true
	renewable = true
	encrypt = true
	krb5_aklog_path = /usr/bin/aklog

	telnet = {
	}

	rcp = {
		forward = true
		encrypt = false
		allow_fallback = true
	}

	rsh = {
		allow_fallback = true
	}

	rlogin = {
		allow_fallback = false
	}


	login = {
		forwardable = true
		krb5_run_aklog = false
		krb5_get_tickets = true
		krb4_get_tickets = false
		krb4_convert = false
	}

	kinit = {
		forwardable = true
		krb5_run_aklog = false
	}

	kadmin = {
		forwardable = false
	}

	rshd = {
		krb5_run_aklog = false
	}

	ftpd = {
		krb5_run_aklog = false
		default_lifetime = 10h
	}

	pam = {
		debug = false
		forwardable = true
		renew_lifetime = 7d
		ticket_lifetime = 1560m
		krb4_convert = true
		afs_cells = fnal.gov
		krb5_run_aklog = false
	}
"

test Krb5.lns get fermi_str =
  { "#comment" = "##" }
  { "#comment" = "## This krb5.conf template is intended for use with Fermi" }
  { "#comment" = "## Kerberos v1_2 and later.  Earlier versions may choke on the" }
  { "#comment" = "## \"auth_to_local = \" lines unless they are commented out." }
  { "#comment" = "## The installation process should do all the right things in" }
  { "#comment" = "## any case, but if you are reading this and haven't updated" }
  { "#comment" = "## your kerberos product to v1_2 or later, you really should!" }
  { "#comment" = "##" }
  { "libdefaults"
    { "ticket_lifetime" = "1560m" }
    { "default_realm" = "FNAL.GOV" }
    { "ccache_type" = "4" }
    { "default_tgs_enctypes" = "des-cbc-crc" }
    { "#eol" }
    { "default_tkt_enctypes" = "des-cbc-crc" }
    { "#eol" }
    { "permitted_enctypes" = "des-cbc-crc" }
    { "permitted_enctypes" = "des3-cbc-sha1" }
    { "#eol" }
    { "default_lifetime" = "7d" }
    { "renew_lifetime" = "7d" }
    { "autologin" = "true" }
    { "forward" = "true" }
    { "forwardable" = "true" }
    { "renewable" = "true" }
    { "encrypt" = "true" }
    { "v4_name_convert"
      { "host"
        { "rcmd" = "host" }
      }
    }
    {  } }
  { "realms"
    { "realm" = "FNAL.GOV"
      { "kdc" = "krb-fnal-1.fnal.gov:88" }
      { "kdc" = "krb-fnal-2.fnal.gov:88" }
      { "kdc" = "krb-fnal-3.fnal.gov:88" }
      { "kdc" = "krb-fnal-4.fnal.gov:88" }
      { "kdc" = "krb-fnal-5.fnal.gov:88" }
      { "kdc" = "krb-fnal-6.fnal.gov:88" }
      { "kdc" = "krb-fnal-7.fnal.gov:88" }
      { "master_kdc" = "krb-fnal-admin.fnal.gov:88" }
      { "admin_server" = "krb-fnal-admin.fnal.gov" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "WIN.FNAL.GOV"
      { "kdc" = "littlebird.win.fnal.gov:88" }
      { "kdc" = "bigbird.win.fnal.gov:88" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "FERMI.WIN.FNAL.GOV"
      { "kdc" = "sully.fermi.win.fnal.gov:88" }
      { "kdc" = "elmo.fermi.win.fnal.gov:88" }
      { "kdc" = "grover.fermi.win.fnal.gov:88" }
      { "kdc" = "oscar.fermi.win.fnal.gov:88" }
      { "kdc" = "cookie.fermi.win.fnal.gov:88" }
      { "kdc" = "herry.fermi.win.fnal.gov:88" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "UCHICAGO.EDU"
      { "kdc" = "kerberos-0.uchicago.edu" }
      { "kdc" = "kerberos-1.uchicago.edu" }
      { "kdc" = "kerberos-2.uchicago.edu" }
      { "admin_server" = "kerberos.uchicago.edu" }
      { "default_domain" = "uchicago.edu" } }
    { "realm" = "PILOT.FNAL.GOV"
      { "kdc" = "i-krb-2.fnal.gov:88" }
      { "master_kdc" = "i-krb-2.fnal.gov:88" }
      { "admin_server" = "i-krb-2.fnal.gov" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "WINBETA.FNAL.GOV"
      { "kdc" = "wbdc1.winbeta.fnal.gov:88" }
      { "kdc" = "wbdc2.winbeta.fnal.gov:88" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "FERMIBETA.WINBETA.FNAL.GOV"
      { "kdc" = "fbdc1.fermibeta.winbeta.fnal.gov:88" }
      { "kdc" = "fbdc2.fermibeta.winbeta.fnal.gov:88" }
      { "default_domain" = "fnal.gov" } }
    { "realm" = "CERN.CH"
      { "kdc" = "afsdb2.cern.ch" }
      { "kdc" = "afsdb3.cern.ch" }
      { "kdc" = "afsdb1.cern.ch" }
      { "default_domain" = "cern.ch" }
      { "kpasswd_server" = "afskrb5m.cern.ch" }
      { "admin_server" = "afskrb5m.cern.ch" }
      { "v4_name_convert"
        { "host"
          { "rcmd" = "host" }
        }
      }
    }
    { } }
  { "instancemapping"
    { "afs"
      { "mapping" = "cron/*" { "value" = "" } }
      { "mapping" = "cms/*"  { "value" = "" } }
      { "mapping" = "afs/*"  { "value" = "" } }
      { "mapping" = "e898/*" { "value" = "" } } }
    { } }
  { "capaths"
    {  }
    { "#comment" = "FNAL.GOV and PILOT.FNAL.GOV are the MIT Kerberos Domains" }
    { "#comment" = "FNAL.GOV is production and PILOT is for testing" }
    { "#comment" = "The FERMI Windows domain uses the WIN.FNAL.GOV root realm" }
    { "#comment" = "with the FERMI.WIN.FNAL.GOV sub-realm where machines and users" }
    { "#comment" = "reside.  The WINBETA and FERMIBETA domains are the equivalent" }
    { "#comment" = "testing realms for the FERMIBETA domain.  The 2-way transitive" }
    { "#comment" = "trust structure of this complex is as follows:" }
    {}
    { "#comment" = "FNAL.GOV <=> PILOT.FNAL.GOV" }
    { "#comment" = "FNAL.GOV <=> WIN.FERMI.GOV <=> FERMI.WIN.FERMI.GOV" }
    { "#comment" = "PILOT.FNAL.GOV <=> WINBETA.FNAL.GOV <=> FERMIBETA.WINBETA.FNAL.GOV" }
    {  }
    { "FNAL.GOV"
      { "PILOT.FNAL.GOV" = "." }
      { "FERMI.WIN.FNAL.GOV" = "WIN.FNAL.GOV" }
      { "WIN.FNAL.GOV" = "." }
      { "FERMIBETA.WINBETA.FNAL.GOV" = "WINBETA.FNAL.GOV" }
      { "WINBETA.FNAL.GOV" = "PILOT.FNAL.GOV" } }
    { "PILOT.FNAL.GOV"
      { "FNAL.GOV" = "." }
      { "FERMI.WIN.FNAL.GOV" = "WIN.FNAL.GOV" }
      { "WIN.FNAL.GOV" = "FNAL.GOV" }
      { "FERMIBETA.WINBETA.FNAL.GOV" = "WINBETA.FNAL.GOV" }
      { "WINBETA.FNAL.GOV" = "." } }
    { "WIN.FNAL.GOV"
      { "FNAL.GOV" = "." }
      { "PILOT.FNAL.GOV" = "FNAL.GOV" }
      { "FERMI.WIN.FNAL.GOV" = "." }
      { "FERMIBETA.WINBETA.FNAL.GOV" = "WINBETA.FNAL.GOV" }
      { "WINBETA.FNAL.GOV" = "PILOT.FNAL.GOV" } }
    { "WINBETA.FNAL.GOV"
      { "PILOT.FNAL.GOV" = "." }
      { "FERMIBETA.WINBETA.FNAL.GOV" = "." }
      { "FNAL.GOV" = "PILOT.FNAL.GOV" }
      { "FERMI.WIN.FNAL.GOV" = "WIN.FNAL.GOV" }
      { "WIN.FNAL.GOV" = "PILOT.FNAL.GOV" } }
    { } }
  { "logging"
    { "kdc"
      { "syslog"
        { "severity" = "info" }
        { "facility" = "local1" } } }
    { "admin_server"
      { "syslog"
        { "severity" = "info" }
        { "facility" = "local2" } } }
    { "default"
      { "syslog"
        { "severity" = "err" }
        { "facility" = "auth" } } }
    {  } }
  { "domain_realm"
    { "#comment" = "Fermilab's (non-windows-centric) domains" }
    { ".fnal.gov" = "FNAL.GOV" }
    { ".cdms-soudan.org" = "FNAL.GOV" }
    { ".deemz.net" = "FNAL.GOV" }
    { ".dhcp.fnal.gov" = "FNAL.GOV" }
    { ".minos-soudan.org" = "FNAL.GOV" }
    { "i-krb-2.fnal.gov" = "PILOT.FNAL.GOV" }
    { ".win.fnal.gov" = "WIN.FNAL.GOV" }
    { ".fermi.win.fnal.gov" = "FERMI.WIN.FNAL.GOV" }
    { ".winbeta.fnal.gov" = "WINBETA.FNAL.GOV" }
    { ".fermibeta.winbeta.fnal.gov" = "FERMIBETA.WINBETA.FNAL.GOV" }
    { "#comment" = "Fermilab's KCA servers so FERMI.WIN principals work in FNAL.GOV realm" }
    { "#comment" = "winserver.fnal.gov = FERMI.WIN.FNAL.GOV" }
    { "#comment" = "winserver2.fnal.gov = FERMI.WIN.FNAL.GOVA" }
    { "#comment" = "Accelerator nodes to FERMI.WIN for Linux/OS X users" }
    { "adgroups.fnal.gov" = "FERMI.WIN.FNAL.GOV" }
    { "adusers.fnal.gov" = "FERMI.WIN.FNAL.GOV" }
    { "webad.fnal.gov" = "FERMI.WIN.FNAL.GOV" }
    { "#comment" = "Friends and family (by request)" }
    { ".cs.ttu.edu" = "FNAL.GOV" }
    { ".geol.uniovi.es" = "FNAL.GOV" }
    { ".harvard.edu" = "FNAL.GOV" }
    { ".hpcc.ttu.edu" = "FNAL.GOV" }
    { ".infn.it" = "FNAL.GOV" }
    { ".knu.ac.kr" = "FNAL.GOV" }
    { ".lns.mit.edu" = "FNAL.GOV" }
    { ".ph.liv.ac.uk" = "FNAL.GOV" }
    { ".pha.jhu.edu" = "FNAL.GOV" }
    { ".phys.ttu.edu" = "FNAL.GOV" }
    { ".phys.ualberta.ca" = "FNAL.GOV" }
    { ".physics.lsa.umich.edu" = "FNAL.GOV" }
    { ".physics.ucla.edu" = "FNAL.GOV" }
    { ".physics.ucsb.edu" = "FNAL.GOV" }
    { ".physics.utoronto.ca" = "FNAL.GOV" }
    { ".rl.ac.uk" = "FNAL.GOV" }
    { ".rockefeller.edu" = "FNAL.GOV" }
    { ".rutgers.edu" = "FNAL.GOV" }
    { ".sdsc.edu" = "FNAL.GOV" }
    { ".sinica.edu.tw" = "FNAL.GOV" }
    { ".tsukuba.jp.hep.net" = "FNAL.GOV" }
    { ".ucsd.edu" = "FNAL.GOV" }
    { ".unl.edu" = "FNAL.GOV" }
    { ".in2p3.fr" = "FNAL.GOV" }
    { ".wisc.edu" = "FNAL.GOV" }
    { ".pic.org.es" = "FNAL.GOV" }
    { ".kisti.re.kr" = "FNAL.GOV" }
    {  }
    { "#comment" = "The whole \"top half\" is replaced during \"ups installAsRoot krb5conf\", so:" }
    { "#comment" = "It would probably be a bad idea to change anything on or above this line" }
    {  }
    { "#comment" = "If you need to add any .domains or hosts, put them here" } }
  { "domain_realm"
    { "mojo.lunet.edu" = "FNAL.GOV" }
    {  } }
  { "appdefaults"
    { "default_lifetime" = "7d" }
    { "retain_ccache" = "false" }
    { "autologin" = "true" }
    { "forward" = "true" }
    { "forwardable" = "true" }
    { "renewable" = "true" }
    { "encrypt" = "true" }
    { "krb5_aklog_path" = "/usr/bin/aklog" }
    {  }
    { "application" = "telnet" }
    {  }
    { "application" = "rcp"
      { "forward" = "true" }
      { "encrypt" = "false" }
      { "allow_fallback" = "true" } }
    {  }
    { "application" = "rsh"
      { "allow_fallback" = "true" } }
    {  }
    { "application" = "rlogin"
      { "allow_fallback" = "false" } }
    {  }
    {  }
    { "application" = "login"
      { "forwardable" = "true" }
      { "krb5_run_aklog" = "false" }
      { "krb5_get_tickets" = "true" }
      { "krb4_get_tickets" = "false" }
      { "krb4_convert" = "false" } }
    {  }
    { "application" = "kinit"
      { "forwardable" = "true" }
      { "krb5_run_aklog" = "false" } }
    {  }
    { "application" = "kadmin"
      { "forwardable" = "false" } }
    {  }
    { "application" = "rshd"
      { "krb5_run_aklog" = "false" } }
    {  }
    { "application" = "ftpd"
      { "krb5_run_aklog" = "false" }
      { "default_lifetime" = "10h" } }
    {  }
    { "application" = "pam"
      { "debug" = "false" }
      { "forwardable" = "true" }
      { "renew_lifetime" = "7d" }
      { "ticket_lifetime" = "1560m" }
      { "krb4_convert" = "true" }
      { "afs_cells" = "fnal.gov" }
      { "krb5_run_aklog" = "false" } } }


(* Example from the krb5 distrubution *)
let dist_str = "[libdefaults]
	default_realm = ATHENA.MIT.EDU
	krb4_config = /usr/kerberos/lib/krb.conf
	krb4_realms = /usr/kerberos/lib/krb.realms

[realms]
	ATHENA.MIT.EDU = {
		admin_server = KERBEROS.MIT.EDU
		default_domain = MIT.EDU
		v4_instance_convert = {
			mit = mit.edu
			lithium = lithium.lcs.mit.edu
		}
	}
	ANDREW.CMU.EDU = {
		admin_server = vice28.fs.andrew.cmu.edu
	}
# use \"kdc =\" if realm admins haven't put SRV records into DNS
        GNU.ORG = {
                kdc = kerberos.gnu.org
                kdc = kerberos-2.gnu.org
                admin_server = kerberos.gnu.org
        }

[domain_realm]
	.mit.edu = ATHENA.MIT.EDU
	mit.edu = ATHENA.MIT.EDU
	.media.mit.edu = MEDIA-LAB.MIT.EDU
	media.mit.edu = MEDIA-LAB.MIT.EDU
	.ucsc.edu = CATS.UCSC.EDU

[logging]
#	kdc = CONSOLE
"

test Krb5.lns get dist_str =
  { "libdefaults"
      { "default_realm" = "ATHENA.MIT.EDU" }
      { "krb4_config" = "/usr/kerberos/lib/krb.conf" }
      { "krb4_realms" = "/usr/kerberos/lib/krb.realms" }
      { } }
    { "realms"
        { "realm" = "ATHENA.MIT.EDU"
            { "admin_server" = "KERBEROS.MIT.EDU" }
            { "default_domain" = "MIT.EDU" }
            { "v4_instance_convert"
                { "mit" = "mit.edu" }
                { "lithium" = "lithium.lcs.mit.edu" } } }
        { "realm" = "ANDREW.CMU.EDU"
            { "admin_server" = "vice28.fs.andrew.cmu.edu" } }
        { "#comment" = "use \"kdc =\" if realm admins haven't put SRV records into DNS" }
        { "realm" = "GNU.ORG"
            { "kdc" = "kerberos.gnu.org" }
            { "kdc" = "kerberos-2.gnu.org" }
            { "admin_server" = "kerberos.gnu.org" } }
        { } }
    { "domain_realm"
        { ".mit.edu" = "ATHENA.MIT.EDU" }
        { "mit.edu" = "ATHENA.MIT.EDU" }
        { ".media.mit.edu" = "MEDIA-LAB.MIT.EDU" }
        { "media.mit.edu" = "MEDIA-LAB.MIT.EDU" }
        { ".ucsc.edu" = "CATS.UCSC.EDU" }
        { } }
    { "logging"
        { "#comment" = "kdc = CONSOLE" } }

(* Test for [libdefaults] *)
test Krb5.libdefaults get "[libdefaults]
	default_realm = ATHENA.MIT.EDU
	krb4_config = /usr/kerberos/lib/krb.conf
	krb4_realms = /usr/kerberos/lib/krb.realms\n\n" =
  { "libdefaults"
    { "default_realm" = "ATHENA.MIT.EDU" }
    { "krb4_config" = "/usr/kerberos/lib/krb.conf" }
    { "krb4_realms" = "/usr/kerberos/lib/krb.realms" }
    { } }

(* Test for [appfdefaults] *)
test Krb5.appdefaults get "[appdefaults]\n\tdefault_lifetime = 7d\n" =
  { "appdefaults" { "default_lifetime" = "7d" } }

test Krb5.appdefaults get
 "[appdefaults]\nrcp = { \n forward = true\n  encrypt = false\n  }\n" =
  { "appdefaults"
      { "application" = "rcp"
          { "forward" = "true" }
          { "encrypt" = "false" } } }

test Krb5.appdefaults get "[appdefaults]\ntelnet = {\n\t}\n" =
  { "appdefaults" { "application" = "telnet" } }

test Krb5.appdefaults get  "[appdefaults]
  rcp = {
    forward = true
    ATHENA.MIT.EDU = {
      encrypt = false
    }
    MEDIA-LAB.MIT.EDU = {
      encrypt = true
    }
    forwardable = true
  }\n"  =
  { "appdefaults"
      { "application" = "rcp"
          { "forward" = "true" }
          { "realm" = "ATHENA.MIT.EDU"
              { "encrypt" = "false" } }
          { "realm" = "MEDIA-LAB.MIT.EDU"
              { "encrypt" = "true" } }
          { "forwardable" = "true" } } }

let appdef = "[appdefaults]
	default_lifetime = 7d
	retain_ccache = false
	autologin = true
	forward = true
	forwardable = true
	renewable = true
	encrypt = true
	krb5_aklog_path = /usr/bin/aklog

	telnet = {
	}

	rcp = {
		forward = true
		encrypt = false
		allow_fallback = true
	}

	rsh = {
		allow_fallback = true
	}

	rlogin = {
		allow_fallback = false
	}


	login = {
		forwardable = true
		krb5_run_aklog = false
		krb5_get_tickets = true
		krb4_get_tickets = false
		krb4_convert = false
	}

	kinit = {
		forwardable = true
		krb5_run_aklog = false
	}

	kadmin = {
		forwardable = false
	}

	rshd = {
		krb5_run_aklog = false
	}

	ftpd = {
		krb5_run_aklog = false
		default_lifetime = 10h
	}

	pam = {
		debug = false
		forwardable = true
		renew_lifetime = 7d
		ticket_lifetime = 1560m
		krb4_convert = true
		afs_cells = fnal.gov
		krb5_run_aklog = false
	}\n"

let appdef_tree =
  { "appdefaults"
    { "default_lifetime" = "7d" }
    { "retain_ccache" = "false" }
    { "autologin" = "true" }
    { "forward" = "true" }
    { "forwardable" = "true" }
    { "renewable" = "true" }
    { "encrypt" = "true" }
    { "krb5_aklog_path" = "/usr/bin/aklog" }
    {  }
    { "application" = "telnet" }
    {  }
    { "application" = "rcp"
      { "forward" = "true" }
      { "encrypt" = "false" }
      { "allow_fallback" = "true" }
    }
    {  }
    { "application" = "rsh"
      { "allow_fallback" = "true" }
    }
    {  }
    { "application" = "rlogin"
      { "allow_fallback" = "false" }
    }
    {  }
    {  }
    { "application" = "login"
      { "forwardable" = "true" }
      { "krb5_run_aklog" = "false" }
      { "krb5_get_tickets" = "true" }
      { "krb4_get_tickets" = "false" }
      { "krb4_convert" = "false" }
    }
    {  }
    { "application" = "kinit"
      { "forwardable" = "true" }
      { "krb5_run_aklog" = "false" }
    }
    {  }
    { "application" = "kadmin"
      { "forwardable" = "false" }
    }
    {  }
    { "application" = "rshd"
      { "krb5_run_aklog" = "false" }
    }
    {  }
    { "application" = "ftpd"
      { "krb5_run_aklog" = "false" }
      { "default_lifetime" = "10h" }
    }
    {  }
    { "application" = "pam"
      { "debug" = "false" }
      { "forwardable" = "true" }
      { "renew_lifetime" = "7d" }
      { "ticket_lifetime" = "1560m" }
      { "krb4_convert" = "true" }
      { "afs_cells" = "fnal.gov" }
      { "krb5_run_aklog" = "false" }
    }
  }


test Krb5.appdefaults get appdef = appdef_tree
test Krb5.lns get appdef = appdef_tree


(* Test realms section *)
let realms_str = "[realms]
   ATHENA.MIT.EDU = {
        admin_server = KERBEROS.MIT.EDU
        default_domain = MIT.EDU
        database_module = ldapconf

        # test
        v4_instance_convert = {
             mit = mit.edu
             lithium = lithium.lcs.mit.edu
        }
        v4_realm = LCS.MIT.EDU
   }\n"

test Krb5.lns get realms_str =
  { "realms"
    { "realm" = "ATHENA.MIT.EDU"
      { "admin_server" = "KERBEROS.MIT.EDU" }
      { "default_domain" = "MIT.EDU" }
      { "database_module" = "ldapconf" }
      { }
      { "#comment" = "test" }
      { "v4_instance_convert"
        { "mit" = "mit.edu" }
        { "lithium" = "lithium.lcs.mit.edu" } }
      { "v4_realm" = "LCS.MIT.EDU" } } }

(* Test dpmain_realm section *)
let domain_realm_str = "[domain_realm]
    .mit.edu = ATHENA.MIT.EDU
    mit.edu = ATHENA.MIT.EDU
    dodo.mit.edu = SMS_TEST.MIT.EDU
    .ucsc.edu = CATS.UCSC.EDU\n"

test Krb5.lns get domain_realm_str =
  { "domain_realm"
      { ".mit.edu" = "ATHENA.MIT.EDU" }
      { "mit.edu" = "ATHENA.MIT.EDU" }
      { "dodo.mit.edu" = "SMS_TEST.MIT.EDU" }
      { ".ucsc.edu" = "CATS.UCSC.EDU" } }

(* Test logging section *)
let logging_str = "[logging]
    kdc = CONSOLE
    kdc = SYSLOG:INFO:DAEMON
    admin_server = FILE:/var/adm/kadmin.log
    admin_server = DEVICE=/dev/tty04\n"

test Krb5.lns get logging_str =
  { "logging"
      { "kdc"
          { "console" } }
      { "kdc"
          { "syslog"
              { "severity" = "INFO" }
              { "facility" = "DAEMON" } } }
      { "admin_server"
          { "file" = "/var/adm/kadmin.log" } }
      { "admin_server"
          { "device" = "/dev/tty04" } } }

(* Test capaths section *)
let capaths_str = "[capaths]
    ANL.GOV = {
         TEST.ANL.GOV = .
         PNL.GOV = ES.NET
         NERSC.GOV = ES.NET
         ES.NET = .
    }
    TEST.ANL.GOV = {
         ANL.GOV = .
    }
    PNL.GOV = {
         ANL.GOV = ES.NET
    }
    NERSC.GOV = {
         ANL.GOV = ES.NET
    }
    ES.NET = {
         ANL.GOV = .
    }\n"

test Krb5.lns get capaths_str =
  { "capaths"
      { "ANL.GOV"
          { "TEST.ANL.GOV" = "." }
          { "PNL.GOV" = "ES.NET" }
          { "NERSC.GOV" = "ES.NET" }
          { "ES.NET" = "." } }
      { "TEST.ANL.GOV"
          { "ANL.GOV" = "." } }
      { "PNL.GOV"
          { "ANL.GOV" = "ES.NET" } }
      { "NERSC.GOV"
          { "ANL.GOV" = "ES.NET" } }
      { "ES.NET"
          { "ANL.GOV" = "." } } }

(* Test instancemapping *)

test Krb5.instance_mapping get "[instancemapping]
 afs = {
 	cron/* = \"\"
 	cms/* = \"\"
 	afs/* = \"\"
 	e898/* = \"\"
 }\n" =
  { "instancemapping"
      { "afs"
          { "mapping" = "cron/*"
              { "value" = "" } }
          { "mapping" = "cms/*"
              { "value" = "" } }
          { "mapping" = "afs/*"
              { "value" = "" } }
          { "mapping" = "e898/*"
              { "value" = "" } } } }

test Krb5.kdc get "[kdc]
 profile = /var/kerberos/krb5kdc/kdc.conf\n" =
  { "kdc"
    { "profile" = "/var/kerberos/krb5kdc/kdc.conf" } }

(* v4_name_convert in libdefaults *)
test Krb5.libdefaults get "[libdefaults]
        default_realm = MY.REALM
	clockskew = 300
	v4_instance_resolve = false
	v4_name_convert = {
		host = {
			rcmd = host
			ftp = ftp
		}
		plain = {
			something = something-else
		}
	}\n" =

  { "libdefaults"
    { "default_realm" = "MY.REALM" }
    { "clockskew" = "300" }
    { "v4_instance_resolve" = "false" }
    { "v4_name_convert"
      { "host" { "rcmd" = "host" } { "ftp" = "ftp" } }
      { "plain" { "something" = "something-else" } } } }

(* Test pam section *)
let pam_str = "[pam]
 debug = false
 ticket_lifetime = 36000
 renew_lifetime = 36000
 forwardable = true
 krb4_convert = false
"

test Krb5.lns get pam_str =
  { "pam"
      { "debug" = "false" }
      { "ticket_lifetime" = "36000" }
      { "renew_lifetime" = "36000" }
      { "forwardable" = "true" }
      { "krb4_convert" = "false" } }

(* Ticket #274 - multiple *enctypes values *)
let multiple_enctypes = "[libdefaults]
permitted_enctypes = arcfour-hmac-md5 arcfour-hmac des3-cbc-sha1 des-cbc-md5 des-cbc-crc aes128-cts
default_tgs_enctypes = des3-cbc-sha1 des-cbc-md5
default_tkt_enctypes = des-cbc-md5
"

test Krb5.lns get multiple_enctypes =
  { "libdefaults"
    { "permitted_enctypes" = "arcfour-hmac-md5" }
    { "permitted_enctypes" = "arcfour-hmac" }
    { "permitted_enctypes" = "des3-cbc-sha1" }
    { "permitted_enctypes" = "des-cbc-md5" }
    { "permitted_enctypes" = "des-cbc-crc" }
    { "permitted_enctypes" = "aes128-cts" }
    { "#eol" }
    { "default_tgs_enctypes" = "des3-cbc-sha1" }
    { "default_tgs_enctypes" = "des-cbc-md5" }
    { "#eol" }
    { "default_tkt_enctypes" = "des-cbc-md5" }
    { "#eol" }
  }

(* Ticket #274 - v4_name_convert subsection *)
let v4_name_convert = "[realms]
 EXAMPLE.COM = {
  kdc = kerberos.example.com:88
  admin_server = kerberos.example.com:749
  default_domain = example.com
  ticket_lifetime = 12h
  v4_name_convert = {
     host = {
       rcmd = host
     }
  }
 }
"

test Krb5.lns get v4_name_convert =
  { "realms"
    { "realm" = "EXAMPLE.COM"
      { "kdc" = "kerberos.example.com:88" }
      { "admin_server" = "kerberos.example.com:749" }
      { "default_domain" = "example.com" }
      { "ticket_lifetime" = "12h" }
      { "v4_name_convert"
        { "host"
          { "rcmd" = "host" }
        }
      }
    }
  }

(* Ticket #288: semicolons for comments *)
test Krb5.lns get "; AD  : This Kerberos configuration is for CERN's Active Directory realm.\n" =
    { "#comment" = "AD  : This Kerberos configuration is for CERN's Active Directory realm." }
