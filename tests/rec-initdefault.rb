# Change the initdefault
#
# Note that we would really like to query for the node whose 'action' is
# initdefault and then set its 'runlevels' sibling. That's not possible,
# mostly because the testing framework and augtool are too simplistic to do
# that. Would be easy with a shell script

# Query for the record that holds the initdefault
record=`#{AUGTOOL} match '/system/config/inittab/*/action' initdefault`.chomp
record = File::dirname(record)

commands="
set #{record}/runlevels 3
save
"

diff["/etc/inittab"] = <<TXT
--- /tmp/aug/etc/inittab
+++ /tmp/aug/etc/inittab.augnew
@@ -15,7 +15,7 @@
 #   5 - X11
 #   6 - reboot (Do NOT set initdefault to this)
 # 
-id:5:initdefault:
+id:3:initdefault:
 
 # System initialization.
 si::sysinit:/etc/rc.d/rc.sysinit
TXT

