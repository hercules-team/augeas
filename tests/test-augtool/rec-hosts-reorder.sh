# Delete the first hosts entry and then add it back into the tree
# Because of how nodes are ordered, this should be equivalent to
# moving the corresponding line to the end of the file
commands="
rm /files/etc/hosts/1
set /files/etc/hosts/1/ipaddr 127.0.0.1
set /files/etc/hosts/1/canonical localhost.localdomain
set /files/etc/hosts/1/alias[1] localhost
set /files/etc/hosts/1/alias[2] galia.watzmann.net
set /files/etc/hosts/1/alias[3] galia
"

lens=Hosts.lns
file="/etc/hosts"

diff='--- /etc/hosts
+++ /etc/hosts.augnew
@@ -1,6 +1,6 @@
 # Do not remove the following line, or various programs
 # that require network functionality will fail.
-127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia
 #172.31.122.254   granny.watzmann.net granny puppet
 #172.31.122.1     galia.watzmann.net galia
 172.31.122.14   orange.watzmann.net orange
+127.0.0.1\tlocalhost.localdomain\tlocalhost galia.watzmann.net galia'
