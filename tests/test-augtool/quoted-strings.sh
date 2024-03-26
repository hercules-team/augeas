#!/bin/bash

#refresh=1
#echo=1
# Notes on quoting in augtool, and augrun
# (does not apply to augset, augmatch etc because they do not rely on augrun to tokenize a command-line)
# shell-style quoting rules apply, eg
#  augtool   value
#   "a b" -> a b
#   "a"b -> ab
#   a" "b -> a b
#   "'"   -> '
#   '"'   -> "
#   '\'' -> '
#   '\"' -> \"
#   "\"" -> "
#   "\'" -> \'
#   "\\" -> \
#   \a   -> unknown escape sequence
#   "\a"   -> unknown escape sequence
#   Except
#   "a\ b"   -> a b
#   "a\tb"   -> a       b  (ie tab)
#   "a\nb"   -> a(newline)b
#   "a\\b"   -> a\b
#
# This file is confusing because bash does additional backslash-quote substitutions, too. Sorry
# Using read ...<<\EOF because bash does not do any interpretion of quotes in this region
# note that within the <<\EOF ... EOF region
#    \\ -> \
#    \\\\ -> \\
# and when a \ is the last char of the line, it needs to be \\ to avoid escaping the trailing newline
# this applies to comments, too
read -d '' commands <<\EOF
set /files/var/tmp/test-quoted-strings.txt/01 '['
set /files/var/tmp/test-quoted-strings.txt/02 ']'
set /files/var/tmp/test-quoted-strings.txt/03 ']['
set /files/var/tmp/test-quoted-strings.txt/04 '[]'
set /files/var/tmp/test-quoted-strings.txt/05 '[]['
set /files/var/tmp/test-quoted-strings.txt/06 '[]]'
set /files/var/tmp/test-quoted-strings.txt/07 '('
set /files/var/tmp/test-quoted-strings.txt/08 ')'
# value is one double-quote
set /files/var/tmp/test-quoted-strings.txt/09 '"'
# value is one single-quote
set /files/var/tmp/test-quoted-strings.txt/10 "'"
# value is one back-slash
set /files/var/tmp/test-quoted-strings.txt/11 '\\\\'
set /files/var/tmp/test-quoted-strings.txt/12 '-- nothing should appear after this line --'
insert 001 after /files/var/tmp/test-quoted-strings.txt/seq::*[.=~regexp('.*\\]\\]\\[ \\)\\( .*')]
set /files/var/tmp/test-quoted-strings.txt/001 'found ]][ )('
insert 002 after /files/var/tmp/test-quoted-strings.txt/seq::*[.=~regexp('.*\\\\..*')]
set /files/var/tmp/test-quoted-strings.txt/002 'found .'
# Find the line which is '"\\
insert 003 after /files/var/tmp/test-quoted-strings.txt/seq::*[.="find simple quotes: '" + '"\\\\']
set /files/var/tmp/test-quoted-strings.txt/003 "found '"'"\\\\'
# Find the line which is just one double-quote
insert 091 after "/files/var/tmp/test-quoted-strings.txt/seq::*[.='"']"
set /files/var/tmp/test-quoted-strings.txt/091 'found single-quoted " in quoted path'
# Find the line which is just one single-quote
insert 101 after "/files/var/tmp/test-quoted-strings.txt/seq::*[.="'"]"
set /files/var/tmp/test-quoted-strings.txt/101 "found double-quoted ' in quoted path"
# Find the line which is just one back-slash
insert 111 after "/files/var/tmp/test-quoted-strings.txt/seq::*[.="\\\\"]"
set /files/var/tmp/test-quoted-strings.txt/111 'found double-quoted \\\\ in quoted path'
EOF
lens=Simplelines.lns
file="/var/tmp/test-quoted-strings.txt"

read -d '' diff <<\EOF
--- /var/tmp/test-quoted-strings.txt
+++ /var/tmp/test-quoted-strings.txt.augnew
@@ -1,3 +1,21 @@
 find this: ]][ )( . * $ / \\
+found .
+found ]][ )(
 find simple quotes: '"\\
+found '"\\
 ---  original end of file ---
+[
+]
+][
+[]
+[][
+[]]
+(
+)
+"
+found single-quoted " in quoted path
+'
+found double-quoted ' in quoted path
+\\
+found double-quoted \\ in quoted path
+-- nothing should appear after this line --
EOF
