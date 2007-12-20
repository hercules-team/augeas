# Parsing /etc/inittab

grammar {

  token SEP ':'

  token WORD /[^:]*(?:|\n)/ = 'none'

  token EOL '\n'

  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) * {
    node $file_name
  }

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: ..? SEP ..? SEP ..? SEP ..? EOL {
    node $seq {
      node 'id' = $1
      node 'runlevels' = $3
      node 'action' = $5
      node 'process' = $7
    }
  }
}