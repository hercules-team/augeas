commands="
set /files/etc/ssh/sshd_config/AcceptEnv[3]/01 FOO
"

lens=Sshd.lns
file="/etc/ssh/sshd_config"

diff='--- /etc/ssh/sshd_config
+++ /etc/ssh/sshd_config.augnew
@@ -93,7 +93,7 @@
 # Accept locale-related environment variables
 AcceptEnv LANG LC_CTYPE LC_NUMERIC LC_TIME LC_COLLATE LC_MONETARY LC_MESSAGES
 AcceptEnv LC_PAPER LC_NAME LC_ADDRESS LC_TELEPHONE LC_MEASUREMENT
-AcceptEnv LC_IDENTIFICATION LC_ALL
+AcceptEnv LC_IDENTIFICATION LC_ALL FOO
 #AllowTcpForwarding yes
 #GatewayPorts no
 #X11Forwarding no'
