commands="
set /files/etc/yum.conf/main/newparam newval
"

lens=Yum.lns
file="/etc/yum.conf"

diff='--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -13,3 +13,4 @@

 # PUT YOUR REPOS HERE OR IN separate files named file.repo
 # in /etc/yum.repos.d
+newparam=newval'
