# The action for @1 tries to store outside of its subtree
grammar store-out-of-tree {
  
  token EOL '\n'
  token EQ /\s*=\s*/ = ' = '

  entry: ... ( EQ ... ) EOL {
    @1 { $3 = $1 }
  }

}
