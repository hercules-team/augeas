#
# sshd_config
#
# This does not address the Match feature yet

grammar {
  token EOL /[ \t]*\n/ = '\n'
  token SEP /[ \t]+(?!\n)/ = ' '

  start: (comment | acceptenv | entry ) * {
    @0 { 'sshd' }
  }

  comment: /(#.*|[ \t]*)\n/

  entry: ... ( SEP ... ) EOL {
    @1 { $1 = $3 }
  }

  acceptenv: 'AcceptEnv' (SEP ...) + EOL {
    @1 { 'accept_env' $seq = $3 }
  }
}
