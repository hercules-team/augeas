# set all existing repos to enabled
commands="
setm /files/etc/yum.repos.d/fedora.repo/* enabled 1
"

lens=Yum.lns
file="/etc/yum.repos.d/fedora.repo"

diff='--- /etc/yum.repos.d/fedora.repo
+++ /etc/yum.repos.d/fedora.repo.augnew
@@ -12,7 +12,7 @@
 failovermethod=priority
 #baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/debug/
 mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-debug-$releasever&arch=$basearch
-enabled=0
+enabled=1
 gpgcheck=1
 gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY

@@ -21,6 +21,6 @@
 failovermethod=priority
 #baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/source/SRPMS/
 mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-source-$releasever&arch=$basearch
-enabled=0
+enabled=1
 gpgcheck=1
 gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY'
