AUGTESTS=$(wildcard *.rb)

GNULIB= ../gnulib/lib/libgnu.la
GNULIB_CFLAGS= -I $(top_srcdir)/gnulib/lib

AM_CFLAGS = $(AUGEAS_CFLAGS) $(WARN_CFLAGS) $(GNULIB_CFLAGS) \
	-I $(top_builddir)/src

valgrind: fatest
	libtool --mode=execute valgrind --quiet --leak-check=full ./fatest

lens_tests =			\
  lens-sudoers.sh		\
  lens-aliases.sh		\
  lens-approx.sh		\
  lens-aptpreferences.sh	\
  lens-aptsource.sh		\
  lens-bbhosts.sh		\
  lens-cgconfig.sh		\
  lens-cgrules.sh  		\
  lens-cobblersettings.sh	\
  lens-cobblermodules.sh	\
  lens-cron.sh			\
  lens-darkice.sh		\
  lens-debctrl.sh		\
  lens-dhclient.sh		\
  lens-dnsmasq.sh		\
  lens-dpkg.sh			\
  lens-dput.sh			\
  lens-ethers.sh		\
  lens-exports.sh		\
  lens-fstab.sh			\
  lens-gdm.sh			\
  lens-group.sh			\
  lens-json.sh          \
  lens-hosts.sh			\
  lens-inetd.sh         \
  lens-inifile.sh		\
  lens-inittab.sh		\
  lens-interfaces.sh		\
  lens-iptables.sh		\
  lens-krb5.sh			\
  lens-ldap.sh			\
  lens-limits.sh		\
  lens-logrotate.sh		\
  lens-lokkit.sh		\
  lens-modprobe.sh		\
  lens-monit.sh			\
  lens-multipath.sh     \
  lens-nagioscfg.sh     \
  lens-ntp.sh			\
  lens-openvpn.sh		\
  lens-pam.sh			\
  lens-passwd.sh		\
  lens-pbuilder.sh		\
  lens-php.sh			\
  lens-phpvars.sh		\
  lens-postfix_access.sh \
  lens-postfix_main.sh		\
  lens-postfix_master.sh	\
  lens-puppet.sh		\
  lens-rsyncd.sh		\
  lens-rx.sh			\
  lens-samba.sh			\
  lens-securetty.sh     \
  lens-services.sh		\
  lens-shellvars.sh		\
  lens-slapd.sh			\
  lens-soma.sh			\
  lens-spacevars.sh		\
  lens-squid.sh			\
  lens-sshd.sh			\
  lens-sysctl.sh		\
  lens-vsftpd.sh		\
  lens-webmin.sh		\
  lens-wine.sh			\
  lens-xinetd.sh		\
  lens-xorg.sh			\
  lens-grub.sh			\
  lens-yum.sh

ME = tests/Makefile.am

# Ensure that the above list stays up to date:
# Construct two lists: list of lens-*.sh from lens_tests = ... above,
# and the list of ../lenses/tests/test_*.aug names.
# If they're not the same, print the new or removed names and fail.
check: check-lens-tests
.PHONY: check-lens-tests
_v = lens_tests
check-lens-tests:
	@u=$$({ sed -n '/^$(_v) =[	 ]*\\$$/,/[^\]$$/p'		\
		$(srcdir)/Makefile.am					\
	    | sed 's/^  *//;/^\$$.*/d;/^$(_v) =/d'			\
	    | sed 's,\.sh.*\\,.sh,';					\
	  ls -1 $(srcdir)/../lenses/tests/test_*.aug			\
	    | sed 's,.*/test_\([^./]*\)\.aug$$,lens-\1.sh,';		\
	} | LC_ALL=C sort | uniq -u);					\
	test "x$$u" = x && :						\
	  || { printf '%s\n' "$$u" >&2;					\
	       echo '$(ME): new test(s)?  update lens_tests' >&2; exit 1; }

DISTCLEANFILES = $(lens_tests)
$(lens_tests): lens-test-1
	$(LN_S) $< $@

check_SCRIPTS = \
  test-interpreter.sh \
  $(lens_tests) \
  test-get.sh \
  test-put-symlink.sh test-save-empty.sh test-mv.sh \
  test-bug-1.sh test-idempotent.sh test-preserve.sh \
  test-events-saved.sh test-save-mode.sh test-unlink-error.sh \
  test-augtool-empty-line.sh

EXTRA_DIST = \
  augtest $(AUGTESTS) root lens-test-1 \
  $(check_SCRIPTS) $(wildcard modules/*.aug) xpath.tests

noinst_SCRIPTS = $(check_SCRIPTS)

check_PROGRAMS = fatest test-xpath test-load test-save test-api

TESTS_ENVIRONMENT = \
  PATH='$(abs_top_builddir)/src$(PATH_SEPARATOR)'"$$PATH" \
  abs_top_builddir='$(abs_top_builddir)' \
  abs_top_srcdir='$(abs_top_srcdir)' \
  LANG=en_US

TESTS = augtest $(check_SCRIPTS) $(check_PROGRAMS)

INCLUDES = -I$(top_srcdir)/src

fatest_SOURCES = fatest.c cutest.c cutest.h $(top_srcdir)/src/memory.c $(top_srcdir)/src/memory.h
fatest_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_xpath_SOURCES = test-xpath.c $(top_srcdir)/src/memory.c
test_xpath_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_load_SOURCES = test-load.c cutest.c cutest.h $(top_srcdir)/src/memory.c $(top_srcdir)/src/memory.h
test_load_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_save_SOURCES = test-save.c cutest.c cutest.h $(top_srcdir)/src/memory.c $(top_srcdir)/src/memory.h
test_save_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

test_api_SOURCES = test-api.c cutest.c cutest.h $(top_srcdir)/src/memory.c $(top_srcdir)/src/memory.h
test_api_LDADD = $(top_builddir)/src/libaugeas.la $(GNULIB)

FAILMALLOC_START ?= 1
FAILMALLOC_REP   ?= 20
FAILMALLOC_PROG ?= ./fatest

include $(top_srcdir)/Makefile.inc
