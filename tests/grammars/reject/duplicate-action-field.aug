grammar {
  token EOL '\n'

  start: 'a' ('b' | 'c' ) EOL {
    @1  { 'group-one' }
    @$2 { 'field-two' }
    @$2 { 'oops' }
  }
}
