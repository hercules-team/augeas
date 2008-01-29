# Parsing /etc/inittab

map {
  grammar pam
  # We really need to be able to exclude some files, like
  # backup files and .rpmsave files
  include '/etc/pam.d/*' '/system/config/pam' $basename
}

grammar pam {

  token SEP /[ \t]+/ = '\t'
  token EOL '\n'
  token CONTROL /(\[[^\]]*\]|[^ \t]+)/ = 'none'
  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) *

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: ... SEP (CONTROL SEP) ... ( SEP ... )? EOL {
    @0  { $seq }
    @$1 { 'type' = $1 }
    @1  { 'control' = $3 }
    @$5 { 'module' = $5 }
    @$7 { 'opts' = $7 }
  }
}
