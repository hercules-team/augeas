# Reference a rule from an action. Not implemented yet
# and leads to failure
grammar action-rule-ref {

  token EOL '\n'

  start: 'a' other {
    @0 { $1 = $2 }
  }
  
  other: 'b' | 'c'
}