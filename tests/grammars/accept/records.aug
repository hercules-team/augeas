# Parsing /etc/hosts -*- augeas -*-
#
#

grammar records {

  token SEP /[ \t]+/ = '\t'

  token OPT_WS /[ \t]*/ = '\t'

  token EOR '\n'

  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) * {
    @0 { $basename }
  }

  comment: POUND_TO_EOL

  record: ... SEP ... ( SEP ... ) * EOR { 
          @0  { $seq }
          @$1 { 'ipaddr' = $1 }
          @$3 { 'canonical' = $3 }
          @1  { 'aliases' $seq = $5 } 
  }

}