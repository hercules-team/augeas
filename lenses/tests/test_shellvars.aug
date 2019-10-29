(* Test for shell lens *)
module Test_shellvars =

  let lns = Shellvars.lns

  let eth_static = "# Intel Corporation PRO/100 VE Network Connection
DEVICE=eth0
BOOTPROTO=static
BROADCAST=172.31.0.255
HWADDR=ab:cd:ef:12:34:56
export IPADDR=172.31.0.31 # this is our IP
#DHCP_HOSTNAME=host.example.com
NETMASK=255.255.255.0
NETWORK=172.31.0.0
unset ONBOOT    #   We do not want this var
"
  let empty_val = "EMPTY=\nDEVICE=eth0\n"

  let key_brack = "SOME_KEY[1]=\nDEVICE=eth0\n"

  test lns get eth_static =
    { "#comment" = "Intel Corporation PRO/100 VE Network Connection" }
    { "DEVICE" = "eth0" }
    { "BOOTPROTO" = "static" }
    { "BROADCAST" = "172.31.0.255" }
    { "HWADDR" = "ab:cd:ef:12:34:56" }
    { "IPADDR" = "172.31.0.31"
        { "export" }
        { "#comment" = "this is our IP" } }
    { "#comment" = "DHCP_HOSTNAME=host.example.com" }
    { "NETMASK" = "255.255.255.0" }
    { "NETWORK" = "172.31.0.0" }
    { "@unset"
        { "1" = "ONBOOT" }
        { "#comment" = "We do not want this var" } }

  test lns put eth_static after
      set "BOOTPROTO" "dhcp" ;
      rm "IPADDR" ;
      rm "BROADCAST" ;
      rm "NETMASK" ;
      rm "NETWORK"
  = "# Intel Corporation PRO/100 VE Network Connection
DEVICE=eth0
BOOTPROTO=dhcp
HWADDR=ab:cd:ef:12:34:56
#DHCP_HOSTNAME=host.example.com
unset ONBOOT    #   We do not want this var
"
  test lns get empty_val =
    { "EMPTY" = "" } { "DEVICE" = "eth0" }

  test lns get key_brack =
    { "SOME_KEY[1]" = "" } { "DEVICE" = "eth0" }

  test lns get "smartd_opts=\"-q never\"\n" =
    { "smartd_opts" = "\"-q never\"" }

  test lns get "var=val  \n" = { "var" = "val" }

  test lns get ". /etc/java/java.conf\n" =
    { ".source" = "/etc/java/java.conf" }

  (* Quoted strings and other oddities *)
  test lns get "var=\"foo 'bar'\"\n" =
    { "var" = "\"foo 'bar'\"" }

  test lns get "var='Some \"funny\" value'\n" =
    { "var" = "'Some \"funny\" value'" }

  test lns get "var=\"\\\"\"\n" =
    { "var" = "\"\\\"\"" }

  test lns get "var=\\\"\n" =
    { "var" = "\\\"" }

  test lns get "var=ab#c\n" =
    { "var" = "ab#c" }

  test lns get "var=ab #c\n" =
    { "var" = "ab"
      { "#comment" = "c" } }

  test lns get "var=ab; #c\n" =
    { "var" = "ab" }
    { "#comment" = "c" }

  test lns put "var=ab; #c\n" after
    set "/#comment" "d" =
    "var=ab; #d\n"

  test lns get "var=ab;\n" =
    { "var" = "ab" }

  test lns get "var='ab#c'\n" =
    { "var" = "'ab#c'" }

  test lns get "var=\"ab#c\"\n" =
    { "var" = "\"ab#c\"" }

  test lns get "ESSID='Joe'\"'\"'s net'\n" =
    { "ESSID" = "'Joe'\"'\"'s net'" }

  test lns get "var=`ab#c`\n" =
    { "var" = "`ab#c`" }

  test lns get "var=`grep nameserver /etc/resolv.conf | head -1`\n" =
    { "var" = "`grep nameserver /etc/resolv.conf | head -1`" }

  test lns put "var=ab #c\n"
    after rm "/var/#comment" = "var=ab\n"

  test lns put "var=ab\n"
    after set "/var/#comment" "this is a var" =
       "var=ab # this is a var\n"

  (* Handling of arrays *)
  test lns get "var=(val1 \"val\\\"2\\\"\" val3)\n" =
    { "var"
        { "1" = "val1" }
        { "2" = "\"val\\\"2\\\"\"" }
        { "3" = "val3" } }

  test lns get "var=()\n" = { "var" = "()" }

  test lns put "var=()\n" after
      set "var" "value"
  = "var=value\n"

  test lns put "var=(v1 v2)\n" after
      rm "var/*" ;
      set "var" "value"
  = "var=value\n"

  test lns put "var=(v1 v2)\n" after
    set "var/3" "v3"
  = "var=(v1 v2 v3)\n"

  test lns get "var=(v1 v2   \n    \t v3)\n" =
  { "var"
    { "1" = "v1" }
    { "2" = "v2" }
    { "3" = "v3" } }

  (* Allow spaces after/before opening/closing parens for array *)
  test lns get "config_eth1=( \"10.128.0.48/24\" )\n" =
  { "config_eth1"  { "1" = "\"10.128.0.48/24\"" } }

  (* Bug 109: allow a bare export *)
  test lns get "export FOO\n" =
  { "@export"
    { "1" = "FOO" } }

  (* Bug 73: allow ulimit builtin *)
  test lns get "ulimit -c unlimited\n" =
  { "@builtin" = "ulimit" { "args" = "-c unlimited" } }

  (* Allow shift builtin *)
  test Shellvars.lns get "shift\nshift 2\n" =
  { "@builtin" = "shift" }
  { "@builtin" = "shift" { "args" = "2" } }

  (* Allow exit builtin *)
  test Shellvars.lns get "exit\nexit 2\n" =
  { "@builtin" = "exit" }
  { "@builtin" = "exit" { "args" = "2" } }

  (* Allow wrapping builtin arguments to multiple lines *)
  test Shellvars.lns get "ulimit -c \\\nunlimited\nulimit \\\n -x 123\n" =
  { "@builtin" = "ulimit" { "args" = "-c \\\nunlimited" } }
  { "@builtin" = "ulimit" { "args" = "-x 123" } }

  (* Test semicolons *)
  test lns get "VAR1=\"this;is;a;test\"\nVAR2=this;\n" =
  { "VAR1" = "\"this;is;a;test\"" }
  { "VAR2" = "this" }

  (* Bug 230: parse conditions *)
  test lns get "if [ -f /etc/default/keyboard ]; then\n. /etc/default/keyboard\nfi\n" =
  { "@if" = "[ -f /etc/default/keyboard ]" { ".source" = "/etc/default/keyboard" } }

  (* Recursive condition *)
  test lns get "if [ -f /tmp/file1 ]; then
  if [ -f /tmp/file2 ]
  then
    . /tmp/file2
  elif [ -f /tmp/file3 ]; then
    . /tmp/file3; else; . /tmp/file4
  fi
else
  . /tmp/file3
fi\n" =
  { "@if" = "[ -f /tmp/file1 ]"
    { "@if" = "[ -f /tmp/file2 ]"
      { ".source" = "/tmp/file2" }
      { "@elif" = "[ -f /tmp/file3 ]"
        { ".source" = "/tmp/file3" } }
      { "@else"
        { ".source" = "/tmp/file4" }
      }
    }
    { "@else"
      { ".source" = "/tmp/file3" }
    }
  }

  (* Multiple elif *)
  test Shellvars.lns get "if [ -f /tmp/file1 ]; then
  . /tmp/file1
  elif [ -f /tmp/file2 ]; then
  . /tmp/file2
  elif [ -f /tmp/file3 ]; then
  . /tmp/file3
  fi\n" =
  { "@if" = "[ -f /tmp/file1 ]"
    { ".source" = "/tmp/file1" }
    { "@elif" = "[ -f /tmp/file2 ]"
      { ".source" = "/tmp/file2" }
    }
    { "@elif" = "[ -f /tmp/file3 ]"
      { ".source" = "/tmp/file3" }
    }
  }


  (* Comment or eol *)
  test lns get "VAR=value # eol-comment\n" =
  { "VAR" = "value"
    { "#comment" = "eol-comment" }
  }

  (* One-liners *)
  test lns get "if [ -f /tmp/file1 ]; then . /tmp/file1; else . /tmp/file2; fi\n" =
  { "@if" = "[ -f /tmp/file1 ]"
    { ".source" = "/tmp/file1" }
    { "@else"
      { ".source" = "/tmp/file2" }
    }
  }

  (* Loops *)
  test lns get "for f in /tmp/file*; do
  while [ 1 ]; do . $f; done
done\n" =
  { "@for" = "f in /tmp/file*"
    { "@while" = "[ 1 ]"
      { ".source" = "$f" }
    }
  }

  (* Case *)
  test lns get "case $f in
  /tmp/file1)
    . /tmp/file1
    ;;
  /tmp/file2)
    . /tmp/file2
    ;;
  *)
    unset f
    ;;
esac\n" =
  { "@case" = "$f"
    { "@case_entry"
      { "@pattern" = "/tmp/file1" }
      { ".source" = "/tmp/file1" } }
    { "@case_entry"
      { "@pattern" = "/tmp/file2" }
      { ".source" = "/tmp/file2" } }
    { "@case_entry"
      { "@pattern" = "*" }
      { "@unset"
        { "1" = "f" } } } }

  (* Select *)
  test lns get "select i in a b c; do . /tmp/file$i
   done\n" =
  { "@select" = "i in a b c"
    { ".source" = "/tmp/file$i" }
  }

  (* Return *)
  test lns get "return\nreturn 2\n" =
  { "@return" }
  { "@return" = "2" }

  (* Functions *)
  test Shellvars.lns get "foo() {
  . /tmp/bar
  }\n" =
  { "@function" = "foo"
    { ".source" = "/tmp/bar" }
  }

  test Shellvars.lns get "function foo () {
  . /tmp/bar
  }\n" =
  { "@function" = "foo"
    { ".source" = "/tmp/bar" }
  }

  test Shellvars.lns get "foo() (
  . /tmp/bar
  )\n" =
  { "@function" = "foo"
    { ".source" = "/tmp/bar" }
  }

  (* Dollar assignment *)
  test Shellvars.lns get "FOO=$(bar arg)\n" =
  { "FOO" = "$(bar arg)" }

  (* Empty lines before esac *)
  test Shellvars.lns get "case $f in
  a)
    B=C
    ;;

  esac\n" =
  { "@case" = "$f"
    { "@case_entry"
      { "@pattern" = "a" }
      { "B" = "C" } }
    }


  (* Empty lines before a case_entry *)
  test Shellvars.lns get "case $f in

  a)
    B=C
    ;;

  b)
    A=D
    ;;
  esac\n" =
  { "@case" = "$f"
    { "@case_entry"
      { "@pattern" = "a" }
      { "B" = "C" } }
    { "@case_entry"
      { "@pattern" = "b" }
      { "A" = "D" } } }


  (* Comments anywhere *)
  test Shellvars.lns get "case ${INTERFACE} in
# comment before
eth0)
# comment in
OPTIONS=()
;;

# comment before 2
*)
# comment in 2
unset f
;;
# comment after
esac\n" =
  { "@case" = "${INTERFACE}"
    { "#comment" = "comment before" }
    { "@case_entry"
      { "@pattern" = "eth0" }
      { "#comment" = "comment in" }
      { "OPTIONS" = "()" } }
    { "#comment" = "comment before 2" }
    { "@case_entry"
      { "@pattern" = "*" }
      { "#comment" = "comment in 2" }
      { "@unset"
        { "1" = "f" } } }
    { "#comment" = "comment after" } }

  (* Empty case *)
  test Shellvars.lns get "case $a in
  *)
  ;;
  esac\n" =
  { "@case" = "$a"
    { "@case_entry" { "@pattern" = "*" } } }

  (* case variables can be surrounded by double quotes *)
  test Shellvars.lns get "case \"${options}\" in
*debug*)
  shift
  ;;
esac\n" =
  { "@case" = "\"${options}\""
    { "@case_entry"
      { "@pattern" = "*debug*" }
      { "@builtin" = "shift" } } }

  (* Double quoted values can have newlines *)
  test Shellvars.lns get "FOO=\"123\n456\"\n" =
  { "FOO" = "\"123\n456\"" }

  (* Single quoted values can have newlines *)
  test Shellvars.lns get "FOO='123\n456'\n" =
  { "FOO" = "'123\n456'" }

  (* bquoted values can have semi-colons *)
  test Shellvars.lns get "FOO=`bar=date;$bar`\n" =
  { "FOO" = "`bar=date;$bar`" }

  (* dollar-assigned values can have semi-colons *)
  test Shellvars.lns get "FOO=$(bar=date;$bar)\n" =
  { "FOO" = "$(bar=date;$bar)" }

  (* dollar-assigned value in bquot *)
  test Shellvars.lns get "FOO=`echo $(date)`\n" =
  { "FOO" = "`echo $(date)`" }

  (* bquot value in dollar-assigned value *)
  test Shellvars.lns get "FOO=$(echo `date`)\n" =
  { "FOO" = "$(echo `date`)" }

  (* dbquot *)
  test Shellvars.lns get "FOO=``bar``\n" =
  { "FOO" = "``bar``" }

  (* Partial quoting is allowed *)
  test Shellvars.lns get "FOO=\"$bar\"/'baz'/$(quux)$((1 + 2))\n" =
  { "FOO" = "\"$bar\"/'baz'/$(quux)$((1 + 2))" }

  (* unset can be used on wildcard variables *)
  test Shellvars.lns get "unset ${!LC_*}\n" =
  { "@unset"
    { "1" = "${!LC_*}" } }

  (* Empty comment before entries *)
  test Shellvars.lns get "# \nfoo=bar\n" =
  { "foo" = "bar" }

  (* Empty comment after entries *)
  test Shellvars.lns get "foo=bar\n# \n\n" =
  { "foo" = "bar" }

  (* Whitespace between lines *)
  test Shellvars.lns get "DEVICE=eth0\n\nBOOTPROTO=static\n" =
    { "DEVICE" = "eth0" }
    { "BOOTPROTO" = "static" }

  (* Whitespace after line *)
  test Shellvars.lns get "DEVICE=eth0\n\n" =
    { "DEVICE" = "eth0" }

  (* Fails adding variable assignment between comment and blank line *)
  let ins_after_comment = "# foo

"
  test lns put ins_after_comment after
      insa "foo" "#comment" ;
      set "foo" "yes"
  = "# foo\n\nfoo=yes\n"

  (* Make sure to support empty comments *)
  test lns get "# foo
  #
  #
  foo=bar
  #\n" =
    { "#comment" = "foo" }
    { "foo" = "bar" }

  (* Single quotes in arrays, ticket #357 *)
  test lns get "DLAGENTS=('ftp::/usr/bin/curl -fC - --ftp-pasv --retry 3 --retry-delay 3 -o %o %u'
          'scp::/usr/bin/scp -C %u %o')\n" =
    { "DLAGENTS"
      { "1" = "'ftp::/usr/bin/curl -fC - --ftp-pasv --retry 3 --retry-delay 3 -o %o %u'" }
      { "2" = "'scp::/usr/bin/scp -C %u %o'" } }

  (* Accept continued lines in quoted values *)
  test lns get "BLAH=\" \
test \
test2\"\n" =
  { "BLAH" = "\" \\\ntest \\\ntest2\"" }

  (* Export of multiple variables, RHBZ#1033795 *)
  test lns get "export TestVar1 TestVar2\n" =
    { "@export"
      { "1" = "TestVar1" }
      { "2" = "TestVar2" } }

  (* Support ;; on same line as a case statement entry, RHBZ#1033799 *)
  test lns get "case $ARG in
        0) TestVar=\"test0\" ;;
        1) TestVar=\"test1\" ;;
esac\n" =
    { "@case" = "$ARG"
      { "@case_entry"
        { "@pattern" = "0" }
        { "TestVar" = "\"test0\"" } }
      { "@case_entry"
        { "@pattern" = "1" }
        { "TestVar" = "\"test1\"" } } }

  (* case: support ;; on the same line with multiple commands *)
  test lns get "case $ARG in
        0) Foo=0; Bar=1;;
        1)
	   Foo=2
	   Bar=3; Baz=4;;
esac\n" =
    { "@case" = "$ARG"
      { "@case_entry"
        { "@pattern" = "0" }
        { "Foo" = "0" }
        { "Bar" = "1" }
      }
      { "@case_entry"
        { "@pattern" = "1" }
        { "Foo" = "2" }
        { "Bar" = "3" }
        { "Baz" = "4" }
      }
    }

(* Test: Shellvars.lns
     Support `##` bashism in conditions (GH issue #118) *)
test Shellvars.lns get "if [ \"${APACHE_CONFDIR##/etc/apache2-}\" != \"${APACHE_CONFDIR}\" ] ; then
    SUFFIX=\"-${APACHE_CONFDIR##/etc/apache2-}\"
else
    SUFFIX=
fi\n" =
  { "@if" = "[ \"${APACHE_CONFDIR##/etc/apache2-}\" != \"${APACHE_CONFDIR}\" ]"
    { "SUFFIX" = "\"-${APACHE_CONFDIR##/etc/apache2-}\"" }
    { "@else"
      { "SUFFIX" = "" }
    }
  }

  (* Support $(( .. )) arithmetic expansion in variable assignment, RHBZ#1100550 *)
  test lns get "export MALLOC_PERTURB_=$(($RANDOM % 255 + 1))\n" =
    { "MALLOC_PERTURB_" = "$(($RANDOM % 255 + 1))"
      { "export" } }

  (*
   * Github issue 202
   *)
  let starts_with_blank = "\n  \nVAR=value\n"

  test lns get starts_with_blank = { "VAR" = "value" }

  (* It is now possible to insert at the beginning of a file
   * that starts with blank lines *)
  test lns put starts_with_blank after
    insb "#comment" "/*[1]";
    set "/#comment[1]" "a comment" =
    " # a comment\nVAR=value\n"

  (* Modifications of the file lose the blank lines though *)
  test lns put starts_with_blank after
    set "/VAR2" "abc" = "VAR=value\nVAR2=abc\n"

  test lns put starts_with_blank after
    rm "/VAR";
    set "/VAR2" "abc" = "VAR2=abc\n"

  test lns put starts_with_blank after
    rm "/VAR"         = ""

  (* Support associative arrays *)
  test lns get "var[alpha_beta,gamma]=something\n" =
    { "var[alpha_beta,gamma]" = "something" }

  (* GH #188: support more conditions *)
  test Shellvars.lns get "[ -f $FILENAME ]\n" =
    { "@condition" = "-f $FILENAME"
      { "type" = "[" } }

  test Shellvars.lns get "[[ -f $FILENAME ]]\n" =
    { "@condition" = "-f $FILENAME"
      { "type" = "[[" } }

  (* Allow wrapping loop condition to multiple lines *)
  test Shellvars.lns get "for x in foo \\\nbar\\\nbaz; do y=$x; done\n" =
  { "@for" = "x in foo \\\nbar\\\nbaz" { "y" = "$x" } }

  (* Allow quotes in loop conditions *)
  test Shellvars.lns get "for x in \"$@\"; do y=$x; done\n" =
  { "@for" = "x in \"$@\"" { "y" = "$x" } }

  (* case: support quotes and spaces in pattern lists *)
  test lns get "case $ARG in
        \"foo bar\")
           Foo=0
           ;;
        baz | quux)
           Foo=1
           ;;
esac\n" =
    { "@case" = "$ARG"
      { "@case_entry"
        { "@pattern" = "\"foo bar\"" }
        { "Foo" = "0" }
      }
      { "@case_entry"
        { "@pattern" = "baz" }
        { "@pattern" = "quux" }
        { "Foo" = "1" }
      }
    }

  (* eval *)
  test lns get "eval `dircolors`\n" =
    { "@eval" = "`dircolors`" }

  (* alias *)
  test lns get "alias ls='ls $LS_OPTIONS'\n" =
    { "@alias" = "ls" { "value" = "'ls $LS_OPTIONS'" } }

  test lns get "alias ls-options='ls $LS_OPTIONS'\n" =
    { "@alias" = "ls-options" { "value" = "'ls $LS_OPTIONS'" } }

  (* Allow && and || constructs after condition *)
  test Shellvars.lns get "[ -f $FILENAME ] && do this || or that\n" =
  { "@condition" = "-f $FILENAME"
    { "type" = "[" }
    { "@and" = "do this" }
    { "@or" = "or that" } }

(* Test: Shellvars.lns
     Parse (almost) any command *)
test Shellvars.lns get "echo foobar 'and this is baz'
/usr/local/bin/myscript-with-dash_and_underscore.sh with args
echo foo \
bar\n" =
  { "@command" = "echo"
    { "@arg" = "foobar 'and this is baz'" }
  }
  { "@command" = "/usr/local/bin/myscript-with-dash_and_underscore.sh"
    { "@arg" = "with args" }
  }
  { "@command" = "echo"
    { "@arg" = "foo \\\nbar" }
  }

(* Test: Shellvars.lns
     Support pipes in commands *)
test Shellvars.lns get "echo \"$STRING\" | grep foo\n" =
  { "@command" = "echo"
    { "@arg" = "\"$STRING\"" }
    { "@pipe"
      { "@command" = "grep"
        { "@arg" = "foo" } } } }

(* Test: Shellvars.lns
     Support && and || after command
     GH #215 *)
test Shellvars.lns get "grep -q \"Debian\" /etc/issue && echo moo\n" =
  { "@command" = "grep"
    { "@arg" = "-q \"Debian\" /etc/issue" }
    { "@and"
      { "@command" = "echo"
        { "@arg" = "moo" } } } }

test Shellvars.lns get "grep -q \"Debian\" /etc/issue || echo baa\n" =
  { "@command" = "grep"
    { "@arg" = "-q \"Debian\" /etc/issue" }
    { "@or"
      { "@command" = "echo"
        { "@arg" = "baa" } } } }

test Shellvars.lns get "grep -q \"Debian\" /etc/issue && DEBIAN=1\n" =
  { "@command" = "grep"
    { "@arg" = "-q \"Debian\" /etc/issue" }
    { "@and"
      { "DEBIAN" = "1" } } }

test Shellvars.lns get "cat /etc/issue | grep -q \"Debian\" && echo moo || echo baa\n" =
  { "@command" = "cat"
    { "@arg" = "/etc/issue" }
    { "@pipe"
      { "@command" = "grep"
        { "@arg" = "-q \"Debian\"" }
        { "@and"
          { "@command" = "echo"
            { "@arg" = "moo" }
            { "@or"
              { "@command" = "echo"
                { "@arg" = "baa" } } } } } } } }

(* Command-specific environment variables *)
test Shellvars.lns get "abc=def \\\n  ghi=\"jkl mno\" command arg1 arg2\n" =
  { "@command" = "command"
    { "abc" = "def" }
    { "ghi" = "\"jkl mno\"" }
    { "@arg" = "arg1 arg2" }
  }

(* Wrapped command sequences *)

test Shellvars.lns get "foo && \\\nbar baz \\\n|| qux \\\n    quux\\\ncorge  grault\n" =
  { "@command" = "foo"
    { "@and"
      { "@command" = "bar"
        { "@arg" = "baz" }
	{ "@or" { "@command" = "qux" { "@arg" = "quux\\\ncorge  grault" } } }
      }
    }
  }

(* Comment after function definition (Issue #339) *)
test Shellvars.lns get "SetDir() # hello
{
 echo
}\n" =
  { "@function" = "SetDir"
    { "#comment" = "hello" }
    { "@command" = "echo" }
  }

(* Function with new lines *)
test Shellvars.lns get "MyFunc()
{
 echo
}\n" =
  { "@function" = "MyFunc"
    { "@command" = "echo" }
  }

(* Pipe and newline without cl (Issue #339) *)
test Shellvars.lns get "echo |
tr\n" =
  { "@command" = "echo"
    { "@pipe"
      { "@command" = "tr" } } }


(* Subshell (Issue #339) *)
test Shellvars.lns get "{ echo
}\n" =
  { "@subshell"
    { "@command" = "echo" }
  }

(* One-liner function *)
test Shellvars.lns get "MyFunc() { echo; }\n" =
  { "@function" = "MyFunc"
    { "@command" = "echo" }
  }

(* Support and/or in if conditions *)
test Shellvars.lns get "if [ -f /tmp/file1 ] && [ -f /tmp/file2 ] || [ -f /tmp/file3 ]; then
  echo foo
fi
" =
  { "@if" = "[ -f /tmp/file1 ]"
      { "@and" = "[ -f /tmp/file2 ]" }
      { "@or" = "[ -f /tmp/file3 ]" }
      { "@command" = "echo"
          { "@arg" = "foo" }
      }
  }

(* Support variable as command *)
test Shellvars.lns get "$FOO bar\n" =
  { "@command" = "$FOO"
      { "@arg" = "bar" }
  }


(*********************************************************
 * Group: Unsupported syntax                             *
 *                                                       *
 * The following tests are known to be failing currently *
 *********************************************************)

(* Any piping (Issue #343) *)
test Shellvars.lns get "FOO=bar && BAR=foo
echo foo || { echo bar; }
echo FOO | myfunc() { echo bar; }\n" = *


(* Stream redirections (Issue #626 *)
test Shellvars.lns get "echo foo 2>&1 >/dev/null\n" = *


(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
