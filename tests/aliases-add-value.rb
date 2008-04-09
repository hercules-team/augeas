commands="
set /files/etc/aliases/0/values/10000 barbar
save
"

diff["/etc/aliases"] = <<TXT
--- /etc/aliases
+++ /etc/aliases.augnew
@@ -8,7 +8,7 @@
 #
 
 # Basic system aliases -- these MUST be present.
-mailer-daemon:\tpostmaster
+mailer-daemon:\tpostmaster, barbar
 postmaster:\troot
 
 # General redirections for pseudo accounts.
TXT
