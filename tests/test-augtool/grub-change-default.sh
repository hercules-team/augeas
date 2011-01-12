commands="
set /files/etc/grub.conf/default 2
"

lens=Grub.lns
file="/etc/grub.conf"

diff='--- /etc/grub.conf
+++ /etc/grub.conf.augnew
@@ -7,7 +7,7 @@
 #          kernel /vmlinuz-version ro root=/dev/vg00/lv00
 #          initrd /initrd-version.img
 #boot=/dev/sda
-default=0
+default=2
 timeout=5
 splashimage=(hd0,0)/grub/splash.xpm.gz
 hiddenmenu'
