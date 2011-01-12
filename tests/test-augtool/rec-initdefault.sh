# Change the initdefault

commands="
set /files/etc/inittab/*[action = 'initdefault']/runlevels 3
"

lens=Inittab.lns
file="/etc/inittab"

diff='--- /etc/inittab
+++ /etc/inittab.augnew
@@ -15,7 +15,7 @@
 #   5 - X11
 #   6 - reboot (Do NOT set initdefault to this)
 #
-id:5:initdefault:
+id:3:initdefault:

 # System initialization.
 si::sysinit:/etc/rc.d/rc.sysinit'
