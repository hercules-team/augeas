commands="
rm /files/etc/grub.conf/title[2]
"

lens=Grub.lns
file="/etc/grub.conf"

diff='--- /etc/grub.conf
+++ /etc/grub.conf.augnew
@@ -15,10 +15,6 @@
 \troot (hd0,0)
 \tkernel /vmlinuz-2.6.24.4-64.fc8 ro root=/dev/vg00/lv00
 \tinitrd /initrd-2.6.24.4-64.fc8.img
-title Fedora (2.6.24.3-50.fc8)
-\troot (hd0,0)
-\tkernel /vmlinuz-2.6.24.3-50.fc8 ro root=/dev/vg00/lv00
-\tinitrd /initrd-2.6.24.3-50.fc8.img
 title Fedora (2.6.21.7-3.fc8xen)
 \troot (hd0,0)
 \tkernel /xen.gz-2.6.21.7-3.fc8
@@ -28,4 +24,4 @@
 \troot (hd0,0)
 \tkernel /vmlinuz-2.6.24.3-34.fc8 ro root=/dev/vg00/lv00
 \tinitrd /initrd-2.6.24.3-34.fc8.img
-        savedefault
+\tsavedefault'
