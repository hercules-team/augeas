# Parsing inistyle files

#map
#  grammar yum
#  include '/etc/yum.conf' '/system/config/yum'
#end

grammar yum
  token EOL /\n/ = '\n'

  token INDENT /[ \t]+/ = '\t'

  token EQ /\s*=\s*/ =  '='

  token OPT_WS /[ \t]*/ = ''

  file: ( comment | section ) *

  comment: OPT_WS . ( /#.*/ | 'REM' | 'rem' )? . EOL

  section: [ 
             '[' . key ... . ']' . EOL .
             ( comment | kv ) *
           ]

  kv: [ key ... . EQ . store ... . EOL ]

end
