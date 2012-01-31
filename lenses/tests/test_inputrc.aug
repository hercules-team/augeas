(*
Module: Test_Inputrc
  Provides unit tests and examples for the <Inputrc> lens.
*)

module Test_Inputrc =

(* Variable: conf *)
let conf = "# /etc/inputrc - global inputrc for libreadline
# See readline(3readline) and `info rluserman' for more information.

# Be 8 bit clean.
set input-meta on
set output-meta on

# To allow the use of 8bit-characters like the german umlauts, uncomment
# the line below. However this makes the meta key not work as a meta key,
# which is annoying to those which don't need to type in 8-bit characters.

# set convert-meta off

# try to enable the application keypad when it is called.  Some systems
# need this to enable the arrow keys.
# set enable-keypad on

# see /usr/share/doc/bash/inputrc.arrows for other codes of arrow keys

# do not bell on tab-completion
# set bell-style none
# set bell-style visible

# some defaults / modifications for the emacs mode
$if mode=emacs

# allow the use of the Home/End keys
\"\\e[1~\": beginning-of-line
\"\\e[4~\": end-of-line

# allow the use of the Delete/Insert keys
\"\\e[3~\": delete-char
\"\\e[2~\": quoted-insert

# mappings for \"page up\" and \"page down\" to step to the beginning/end
# of the history
# \"\\e[5~\": beginning-of-history
# \"\\e[6~\": end-of-history

# alternate mappings for \"page up\" and \"page down\" to search the history
# \"\\e[5~\": history-search-backward
# \"\\e[6~\": history-search-forward

# mappings for Ctrl-left-arrow and Ctrl-right-arrow for word moving
\"\\e[1;5C\": forward-word
\"\\e[1;5D\": backward-word
\"\\e[5C\": forward-word
\"\\e[5D\": backward-word
\"\\e\\e[C\": forward-word
\"\\e\\e[D\": backward-word

$if term=rxvt
\"\\e[8~\": end-of-line
\"\\eOc\": forward-word
\"\\eOd\": backward-word
$endif

# for non RH/Debian xterm, can't hurt for RH/Debian xterm
# \"\\eOH\": beginning-of-line
# \"\\eOF\": end-of-line

# for freebsd console
# \"\\e[H\": beginning-of-line
# \"\\e[F\": end-of-line

$endif
"

(* Test: Inputrc.lns *)
test Inputrc.lns get conf =
  { "#comment" = "/etc/inputrc - global inputrc for libreadline" }
  { "#comment" = "See readline(3readline) and `info rluserman' for more information." }
  {  }
  { "#comment" = "Be 8 bit clean." }
  { "input-meta" = "on" }
  { "output-meta" = "on" }
  {  }
  { "#comment" = "To allow the use of 8bit-characters like the german umlauts, uncomment" }
  { "#comment" = "the line below. However this makes the meta key not work as a meta key," }
  { "#comment" = "which is annoying to those which don't need to type in 8-bit characters." }
  {  }
  { "#comment" = "set convert-meta off" }
  {  }
  { "#comment" = "try to enable the application keypad when it is called.  Some systems" }
  { "#comment" = "need this to enable the arrow keys." }
  { "#comment" = "set enable-keypad on" }
  {  }
  { "#comment" = "see /usr/share/doc/bash/inputrc.arrows for other codes of arrow keys" }
  {  }
  { "#comment" = "do not bell on tab-completion" }
  { "#comment" = "set bell-style none" }
  { "#comment" = "set bell-style visible" }
  {  }
  { "#comment" = "some defaults / modifications for the emacs mode" }
  { "@if" = "mode=emacs"
    {  }
    { "#comment" = "allow the use of the Home/End keys" }
    { "entry" = "\\e[1~"
      { "mapping" = "beginning-of-line" }
    }
    { "entry" = "\\e[4~"
      { "mapping" = "end-of-line" }
    }
    {  }
    { "#comment" = "allow the use of the Delete/Insert keys" }
    { "entry" = "\\e[3~"
      { "mapping" = "delete-char" }
    }
    { "entry" = "\\e[2~"
      { "mapping" = "quoted-insert" }
    }
    {  }
    { "#comment" = "mappings for \"page up\" and \"page down\" to step to the beginning/end" }
    { "#comment" = "of the history" }
    { "#comment" = "\"\\e[5~\": beginning-of-history" }
    { "#comment" = "\"\\e[6~\": end-of-history" }
    {  }
    { "#comment" = "alternate mappings for \"page up\" and \"page down\" to search the history" }
    { "#comment" = "\"\\e[5~\": history-search-backward" }
    { "#comment" = "\"\\e[6~\": history-search-forward" }
    {  }
    { "#comment" = "mappings for Ctrl-left-arrow and Ctrl-right-arrow for word moving" }
    { "entry" = "\\e[1;5C"
      { "mapping" = "forward-word" }
    }
    { "entry" = "\\e[1;5D"
      { "mapping" = "backward-word" }
    }
    { "entry" = "\\e[5C"
      { "mapping" = "forward-word" }
    }
    { "entry" = "\\e[5D"
      { "mapping" = "backward-word" }
    }
    { "entry" = "\\e\\e[C"
      { "mapping" = "forward-word" }
    }
    { "entry" = "\\e\\e[D"
      { "mapping" = "backward-word" }
    }
    {  }
    { "@if" = "term=rxvt"
      { "entry" = "\\e[8~"
        { "mapping" = "end-of-line" }
      }
      { "entry" = "\\eOc"
        { "mapping" = "forward-word" }
      }
      { "entry" = "\\eOd"
        { "mapping" = "backward-word" }
      }
    }
    {  }
    { "#comment" = "for non RH/Debian xterm, can't hurt for RH/Debian xterm" }
    { "#comment" = "\"\\eOH\": beginning-of-line" }
    { "#comment" = "\"\\eOF\": end-of-line" }
    {  }
    { "#comment" = "for freebsd console" }
    { "#comment" = "\"\\e[H\": beginning-of-line" }
    { "#comment" = "\"\\e[F\": end-of-line" }
    {  }
  }

