#!/bin/bash

commands="
set /files/var/tmp/test-quoted-strings.txt/01 '['
set /files/var/tmp/test-quoted-strings.txt/02 ']'
set /files/var/tmp/test-quoted-strings.txt/03 ']['
set /files/var/tmp/test-quoted-strings.txt/04 '[]'
set /files/var/tmp/test-quoted-strings.txt/05 '[]['
set /files/var/tmp/test-quoted-strings.txt/06 '[]]'
set /files/var/tmp/test-quoted-strings.txt/07 '('
set /files/var/tmp/test-quoted-strings.txt/08 ')'
set /files/var/tmp/test-quoted-strings.txt/09 '"\""'
insert 010 after /files/var/tmp/test-quoted-strings.txt/seq::*[.=~regexp('.*\]\]\[ \)\( .*')]
set /files/var/tmp/test-quoted-strings.txt/010 'found ]][ )('
insert 011 after /files/var/tmp/test-quoted-strings.txt/seq::*[.=~regexp('.*\\\\..*')]
set /files/var/tmp/test-quoted-strings.txt/011 'found .'
"
lens=Simplelines.lns
file="/var/tmp/test-quoted-strings.txt"

diff='--- /var/tmp/test-quoted-strings.txt
+++ /var/tmp/test-quoted-strings.txt.augnew
@@ -1 +1,12 @@
 find this: ]][ )( . * $ / \
+found .
+found ]][ )(
+[
+]
+][
+[]
+[][
+[]]
+(
+)
+"'
