# Parsing inistyle files

map
  grammar yum
  include '/etc/yum.conf' '/system/config/yum'
end

grammar yum

  token EOL /\n/ = '\n'
  token INDENT /[ \t]+/ = '\t'
  token EQ /\s*=\s*/ =  '='
  token COMMENT /[ \t]*(#|REM|rem).*\n/ = '# \n'

  file: ( comment | section ) *

  comment: [ COMMENT ]

  section: [ 
             '[' . key ... . ']' . EOL .
             ( comment | kv ) *
           ]

  kv: [ key ... . EQ . store ... . EOL ]

end
