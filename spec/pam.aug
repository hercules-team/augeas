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
  token CONTROL /(\[[^]#\n]*\]|[^ \t]+)/ = 'none'
  token COMMENT /[ \t]*(#.*)?\n/ = '# \n'
  token WORD /[^# \t\n]+/ = ''
  token OPTS /[^#\n]+/ = ''

  file: ( comment | record ) *

  comment: [ COMMENT ]

  record: [ seq 'record' .
            [ label 'type' . store WORD ] .
            SEP .
            [ label 'control' . store CONTROL] .
            SEP .
            [ label 'module' . store WORD ] .
            ( [ SEP . label 'opts' . store OPTS ] )? .
            EOL
          ]
end
