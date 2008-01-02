# Parsing inistyle files

grammar inistyle {
  token EOL /\n/ = '\n'

  token INDENT /[ \t]+/ = '\t'

  token EQ /\s*=\s*/ =  '='

  token OPT_WS /[ \t]*/ = ''

  file: ( comment | section ) * {
    @0 { 'yum' }
  }

  comment: OPT_WS ( /#.*/ | 'REM' | 'rem' )? EOL

  section: '[' ... ']' EOL ( kv | comment ) * {
    @1 { $2 }
  }

  kv: ... EQ ... EOL {
    @$3 { $1 = $3 }
  }

  # FIXME: need to deal with continuation lines in values
  # Using something like this in the 'kv' rule may work
  # (when referencing rules from actions is implemented)
  value: ... ( /\n[ \t]+/ ... )*

}
