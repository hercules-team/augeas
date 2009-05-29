module Test_dput =

   let conf = "# Example dput.cf that defines the host that can be used
# with dput for uploading.

[DEFAULT]
login = username
method = ftp
hash = md5
allow_unsigned_uploads = 0
run_lintian = 0
run_dinstall = 0
check_version = 0
scp_compress = 0
post_upload_command =
pre_upload_command =
passive_ftp = 1
default_host_non-us =
default_host_main = hebex
allowed_distributions = (?!UNRELEASED)

[hebex]
fqdn = condor.infra.s1.p.fti.net
login = anonymous
method = ftp
incoming = /incoming/hebex
passive_ftp = 0

[dop/desktop]
fqdn = condor.infra.s1.p.fti.net
login = anonymous
method = ftp
incoming = /incoming/dop/desktop
passive_ftp = 0

[jp-non-us]
fqdn = hp.debian.or.jp
incoming = /pub/Incoming/upload-non-US
login = anonymous

# DISABLED due to being repaired currently
#[erlangen]
#fqdn = ftp.uni-erlangen.de
#incoming = /public/pub/Linux/debian/UploadQueue/
#login = anonymous

[ftp-master]
fqdn = ftp-master.debian.org
incoming = /pub/UploadQueue/
login = anonymous
post_upload_command = /usr/bin/mini-dinstall --batch
# And if you want to override one of the defaults, add it here.
# # For example, comment out the next line
# # login = another_username
# # post_upload_command = /path/to/some/script
# # pre_upload_command = /path/to/some/script

"

   test Dput.lns get conf =
      { "#comment" = "Example dput.cf that defines the host that can be used" }
      { "#comment" = "with dput for uploading." }
      {}
      { "target" = "DEFAULT"
         { "login"  = "username" }
         { "method" = "ftp" }
         { "hash"   = "md5" }
         { "allow_unsigned_uploads" = "0" }
         { "run_lintian" = "0" }
         { "run_dinstall" = "0" }
         { "check_version" = "0" }
         { "scp_compress" = "0" }
         { "post_upload_command" }
         { "pre_upload_command" }
         { "passive_ftp" = "1" }
         { "default_host_non-us" }
         { "default_host_main" = "hebex" }
         { "allowed_distributions" = "(?!UNRELEASED)" }
         {} }
      { "target" = "hebex"
         { "fqdn" = "condor.infra.s1.p.fti.net" }
	 { "login" = "anonymous" }
	 { "method" = "ftp" }
	 { "incoming" = "/incoming/hebex" }
	 { "passive_ftp" = "0" }
         {} }
      { "target" = "dop/desktop"
         { "fqdn" = "condor.infra.s1.p.fti.net" }
	 { "login" = "anonymous" }
	 { "method" = "ftp" }
	 { "incoming" = "/incoming/dop/desktop" }
	 { "passive_ftp" = "0" }
         {} }
      { "target" = "jp-non-us"
         { "fqdn" = "hp.debian.or.jp" }
	 { "incoming" = "/pub/Incoming/upload-non-US" }
	 { "login" = "anonymous" }
         {}
         { "#comment" = "DISABLED due to being repaired currently" }
         { "#comment" = "[erlangen]" }
         { "#comment" = "fqdn = ftp.uni-erlangen.de" }
         { "#comment" = "incoming = /public/pub/Linux/debian/UploadQueue/" }
         { "#comment" = "login = anonymous" }
         {} }
      { "target" = "ftp-master"
         { "fqdn" = "ftp-master.debian.org" }
	 { "incoming" = "/pub/UploadQueue/" }
	 { "login" = "anonymous" }
	 { "post_upload_command" = "/usr/bin/mini-dinstall --batch" }
         { "#comment" = "And if you want to override one of the defaults, add it here." }
         { "#comment" = "# For example, comment out the next line" }
         { "#comment" = "# login = another_username" }
         { "#comment" = "# post_upload_command = /path/to/some/script" }
         { "#comment" = "# pre_upload_command = /path/to/some/script" }
	 {} }
