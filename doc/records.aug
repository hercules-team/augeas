# Parsing /etc/hosts

grammar {

  token SEP /[ \t]+/ = '\t'

  token OPT_WS /[ \t]*/ = '\t'

  token EOR '\n'

  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) * {
    node $file_name
  }

  comment: POUND_TO_EOL

  record: ... SEP ... ( SEP ... ) * EOR {
    node $seq = 'foo' {
      node 'ipaddr' = $1
      node 'canonical' = $3
      node 'aliases' = $5
    }
  }
}