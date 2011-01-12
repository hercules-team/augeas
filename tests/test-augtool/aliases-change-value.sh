commands="
set /files/etc/aliases/3/value[2] ruth
"

lens=Aliases.lns
file="/etc/aliases"

diff='--- /etc/aliases
+++ /etc/aliases.augnew
@@ -12,7 +12,7 @@
 postmaster:\troot

 # General redirections for pseudo accounts.
-bin:\t\troot, adm
+bin:\t\troot, ruth
 daemon:\t\troot
 adm:\t\troot'
