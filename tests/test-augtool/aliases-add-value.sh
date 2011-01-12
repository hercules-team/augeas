commands="
ins value after /files/etc/aliases/1/value[last()]
set /files/etc/aliases/1/value[last()] barbar
"

lens=Aliases.lns
file="/etc/aliases"

diff='--- /etc/aliases
+++ /etc/aliases.augnew
@@ -8,7 +8,7 @@
 #

 # Basic system aliases -- these MUST be present.
-mailer-daemon:\tpostmaster
+mailer-daemon:\tpostmaster, barbar
 postmaster:\troot

 # General redirections for pseudo accounts.'
