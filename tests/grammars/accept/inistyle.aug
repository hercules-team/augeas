# Parsing inistyle files

grammar {
  token EOL '\n'

  token INDENT /[ \t]+/ = '\t'

  token EQ /\s+=\s+/ = ' = '

  token OPT_WS /[ \t]*/ = ''

  file: ( comment | section ) *

  comment: OPT_WS ( /#.*$/ | 'REM' | 'rem' )

  section: '[' ... ']' ( kv ) * {
    @1 { $2 }
  }

  kv: ... EQ value {
    @$3 { $1 = $3 }
  }

  value: ... EOL ( INDENT ... EOL )+

}
