# Delete the first hosts entry and then add it back into the tree as
# a new entry.
#
# Since we add the same entry, but under a new key (01 instead of 1), all
# the separators are set to their defaults. That is why the separator
# between localhost.localdomain and localhost changes from a '\t' to a ' '
# If we used the old key '0' to insert back in, we'd have an exact move of
# the line. That is checked by rec-hosts-reorder

commands="
rm /files/etc/hosts/1
set /files/etc/hosts/01/ipaddr 127.0.0.1
set /files/etc/hosts/01/canonical localhost.localdomain
set /files/etc/hosts/01/alias[1] localhost
set /files/etc/hosts/01/alias[2] galia.watzmann.net
set /files/etc/hosts/01/alias[3] galia
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
+127.0.0.1\tlocalhost.localdomain localhost galia.watzmann.net galia'
