# Parsing /etc/inittab

map
  grammar inittab
  include '/etc/inittab' '/system/config/inittab'
end

grammar inittab

  token SEP ':'
  token EOL '\n'

  file: ( comment | record ) *

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: [ seq 'record' . 
            [ label 'id' . store ..? ] .
            SEP .
            [ label 'runlevels' . store ..? ] .
            SEP .
            [ label 'action' . store ..? ] .
            SEP .
            [ label 'process' . store ..? ] .
            EOL
          ]
end
