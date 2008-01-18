# Parsing /etc/inittab

map {
  grammar pam
  # We really need to be able to exclude some files, like
  # backup files and .rpmsave files
  include '/etc/pam.d/*'
}

grammar pam {

  token SEP /[ \t]+/ = '\t'
  token EOL '\n'
  token BRACK /\[[^\]]*\]/ = 'huh?'
  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) * {
    @0 { 'pam' $basename }
  }

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: ... SEP (BRACK | ...) SEP ... ( SEP ... )? EOL {
    @0  { $seq }
    @$1 { 'type' = $1 }
    @$3 { 'control' = $3 }
    @$4 { 'control' = $4 }
    @$6 { 'module' = $6 }
    @$8 { 'opts' = $8 }
  }
}
