commands="
set /files/etc/yum.conf/other/newparam newval
save
"

diff["/etc/yum.conf"] = <<TXT
--- /etc/yum.conf
+++ /etc/yum.conf.augnew
@@ -13,3 +13,5 @@

 # PUT YOUR REPOS HERE OR IN separate files named file.repo
 # in /etc/yum.repos.d
+[other]
+newparam=newval
TXT
