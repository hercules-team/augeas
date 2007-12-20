# Parsing /etc/hosts

grammar {

  token FOO 'foo'

  first: ( first+ | third ) * {
    node '1' = $1 {
      node '1.1' = 'foo' {
        node '1.1.1' = 'one'
        node '1.1.2' = 'two' {
          node '1.1.2.1' = 'val'
        }
      }
      node '1.2' = 'bar' 
    }
    node '2' = 'top'
  }

  third: ... FOO {
    node 'foo' = $1
  }
}