# Parsing /etc/inittab

map
  grammar inittab
  include '/etc/inittab' '/system/config/inittab'
end

grammar inittab

  token SEP ':'
  token EOL '\n'
  token COMMENT /[ \t]*(#.*?)?\n/ = '# \n'
  token VALUE /[^:\n]*/ = ''

  file: ( comment | record ) *

  comment: [ COMMENT ]

  record: [ seq 'record' . 
            [ label 'id' . store VALUE ] .
            SEP .
            [ label 'runlevels' . store VALUE ] .
            SEP .
            [ label 'action' . store VALUE ] .
            SEP .
            [ label 'process' . store VALUE ] .
            EOL
          ]
end
