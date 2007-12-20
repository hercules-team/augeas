# Parsing inistyle files

grammar {
  token EOL '\n'

  token INDENT /[ \t]+/ = '\t'

  token EQ /\s+=\s+/ = ' = '

  token OPT_WS /[ \t]*/ = ''

  file: ( comment | section ) *

  comment: OPT_WS ( /#.*$/ | 'REM' | 'rem' )

  section: '[' ... ']' kv {
    node $2
  }

  kv: ... EQ value {
    node $1 = $3
  }

  value: ... EOL ( INDENT ... EOL )+

}
