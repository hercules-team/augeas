# This will leave /etc/hosts with nothing but comments and whitespace
commands="
rm /files/etc/hosts/1
rm /files/etc/hosts/2
"

lens=Hosts.lns
file="/etc/hosts"

diff='--- /etc/hosts
+++ /etc/hosts.augnew
@@ -1,6 +1,4 @@
 # Do not remove the following line, or various programs
 # that require network functionality will fail.
-127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
-172.31.122.14   orange.watzmann.net orange'
