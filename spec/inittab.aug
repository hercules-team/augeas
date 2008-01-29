# Parsing /etc/inittab

map {
  grammar inittab
  include '/etc/inittab' '/system/config/inittab'
}

grammar inittab {

  token SEP ':'
  token EOL '\n'

  file: ( comment | record ) *

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: ..? SEP ..? SEP ..? SEP ..? EOL {
    @0  { $seq }
    @$1 { 'id' = $1 }
    @$3 { 'runlevels' = $3 }
    @$5 { 'action' = $5 }
    @$7 { 'process' = $7 }
  }
}

# Dual grammar:
# file: ( comment | record ) *
# comment: <e>
# record: [seq ['id' = $1] ['runlevels' = $3] ['action' = $5] ['process' = $7]]
