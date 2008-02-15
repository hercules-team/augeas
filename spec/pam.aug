# Parsing /etc/inittab

map
  grammar pam
  # We really need to be able to exclude some files, like
  # backup files and .rpmsave files
  include '/etc/pam.d/*' '/system/config/pam' $basename
end

grammar pam

  token SEP /[ \t]+/ = '\t'
  token EOL '\n'
  token CONTROL /(\[[^\]]*\]|[^ \t]+)/ = 'none'
  token POUND_TO_EOL /#.*\n/ = '# '

  file: ( comment | record ) *

  comment: ( /#.*?\n/ | /[ \t]*\n/ )

  record: [ seq 'record' .
            [ label 'type' . store ... ] .
            SEP .
            [ label 'control' . store CONTROL] .
            SEP .
            [ label 'module' . store ... ] .
            ( [ SEP . label 'opts' . store ... ] )? .
            EOL
          ]
end
