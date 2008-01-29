# The actions for @1 and for @$3 both try to store $3
grammar duplicate-store {
  
  token EOL '\n'
  token EQ /\s*=\s*/ = ' = '

  entry: ... ( EQ ... ) EOL {
    @1 { $1 = $3 }
    @$3 { 'foo' = $3 }
  }

}
