# The action for @$3 enters 'foo' but then assigns nothing to it.
grammar useless-enter {
  
  token EOL '\n'
  token EQ /\s*=\s*/ = ' = '

  entry: ... ( EQ ... ) EOL {
    @1 { $1 = $3 }
    @$3 { 'foo' }
  }

}
