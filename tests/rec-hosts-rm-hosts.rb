# This will leave /etc/hosts with nothing but comments and whitespace
refresh=true
# We really need a Ruby API instead of this screen scraping.
# Remove all the /system/config/hosts/* entries
commands = ""
`#{AUGTOOL} ls /system/config/hosts`.each do |l|
    f = l.split(' ')
    if f[0] =~ %r{^/[0-9]+}
        commands += "rm /system/config/hosts#{f[0]}\n"
    end
end
commands +="
save
"

diff["/etc/hosts"] = <<TXT
--- /etc/hosts
+++ /etc/hosts.augnew
@@ -1,6 +1,4 @@
 # Do not remove the following line, or various programs
 # that require network functionality will fail.
-127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
-172.31.122.14   orange.watzmann.net orange
TXT

