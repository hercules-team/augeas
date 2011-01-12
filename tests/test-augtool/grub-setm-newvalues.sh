# add console values to multiple kernel entries
commands="
setm /files/etc/grub.conf/*/kernel[. =~ regexp('.*2.6.24.*')] console[last()+1] tty0
setm /files/etc/grub.conf/*/kernel[. =~ regexp('.*2.6.24.*')] console[last()+1] ttyS0,9600n8
"

lens=Grub.lns
file="/etc/grub.conf"

diff='--- /etc/grub.conf
+++ /etc/grub.conf.augnew
@@ -13,11 +13,11 @@
 hiddenmenu
 title Fedora (2.6.24.4-64.fc8)
 \troot (hd0,0)
-\tkernel /vmlinuz-2.6.24.4-64.fc8 ro root=/dev/vg00/lv00
+\tkernel /vmlinuz-2.6.24.4-64.fc8 ro root=/dev/vg00/lv00 console=tty0 console=ttyS0,9600n8
 \tinitrd /initrd-2.6.24.4-64.fc8.img
 title Fedora (2.6.24.3-50.fc8)
 \troot (hd0,0)
-\tkernel /vmlinuz-2.6.24.3-50.fc8 ro root=/dev/vg00/lv00
+\tkernel /vmlinuz-2.6.24.3-50.fc8 ro root=/dev/vg00/lv00 console=tty0 console=ttyS0,9600n8
 \tinitrd /initrd-2.6.24.3-50.fc8.img
 title Fedora (2.6.21.7-3.fc8xen)
 \troot (hd0,0)
@@ -26,6 +26,6 @@
 \tmodule /initrd-2.6.21.7-3.fc8xen.img
 title Fedora (2.6.24.3-34.fc8)
 \troot (hd0,0)
-\tkernel /vmlinuz-2.6.24.3-34.fc8 ro root=/dev/vg00/lv00
+\tkernel /vmlinuz-2.6.24.3-34.fc8 ro root=/dev/vg00/lv00 console=tty0 console=ttyS0,9600n8
 \tinitrd /initrd-2.6.24.3-34.fc8.img
         savedefault'
