(*  Test for AptCacherNGSecurity lens.

About: License
	Copyright 2013 Erik B. Andersen; this file is licenced under the LGPL v2+.

*)
module Test_AptCacherNGSecurity =
	let conf = "
# This file contains confidential data and should be protected with file
# permissions from being read by untrusted users.
#
# NOTE: permissions are fixated with dpkg-statoverride on Debian systems.
# Read its manual page for details.

# Basic authentication with username and password, required to
# visit pages with administrative functionality. Format: username:password

AdminAuth: mooma:moopa
"

	 test AptCacherNGSecurity.lns get conf =
	 		{}
	 		{ "#comment" = "This file contains confidential data and should be protected with file" }
	 		{ "#comment" = "permissions from being read by untrusted users." }
	 		{}
	 		{ "#comment" = "NOTE: permissions are fixated with dpkg-statoverride on Debian systems." }
	 		{ "#comment" = "Read its manual page for details." }
	 		{}
	 		{ "#comment" = "Basic authentication with username and password, required to" }
	 		{ "#comment" = "visit pages with administrative functionality. Format: username:password" }
	 		{}
	 		{ "AdminAuth"
	 			{ "mooma" = "moopa" }
	 		}
