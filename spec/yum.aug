# Parsing inistyle files

map {
  grammar yum
  include '/etc/yum.conf' '/system/config/yum'
}

grammar yum {
  token EOL /\n/ = '\n'

  token INDENT /[ \t]+/ = '\t'

  token EQ /\s*=\s*/ =  '='

  token OPT_WS /[ \t]*/ = ''

  file: ( comment | section ) *

  comment: OPT_WS ( /#.*/ | 'REM' | 'rem' )? EOL

  section: '[' ... ']' EOL ( comment | kv ) * {
    @1 { $2 }
  }

  kv: ... EQ ... EOL {
    @$3 { $1 = $3 }
  }

}
