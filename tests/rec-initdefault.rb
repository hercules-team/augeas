# Change the initdefault
#
# Note that we would really like to query for the node whose 'action' is
# initdefault and then set its 'runlevels' sibling. That's not possible,
# mostly because the testing framework and augtool are too simplistic to do
# that. Would be easy with a shell script

# Query for the record that holds the initdefault
record=""
`#{AUGTOOL} match '/files/etc/inittab/*/action' initdefault`.each do |l|
    if l =~ %r{^(/files/etc/inittab/[0-9]+)}
        record = $1
    end
end
commands="
set #{record}/runlevels 3
save
"

diff["/etc/inittab"] = <<TXT
--- /etc/inittab
+++ /etc/inittab.augnew
@@ -15,7 +15,7 @@
 #   5 - X11
 #   6 - reboot (Do NOT set initdefault to this)
 # 
-id:5:initdefault:
+id:3:initdefault:
 
 # System initialization.
 si::sysinit:/etc/rc.d/rc.sysinit
TXT

