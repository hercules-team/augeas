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
  token SECNAME /[^]]+/ = ''
  token KEY /[^ \t\n=]+/ = ''
  token VALUE /[^\n]+/ = ''

  file: ( comment | section ) *

  comment: [ COMMENT ]

  section: [ 
             '[' . key SECNAME . ']' . EOL .
             ( comment | kv ) *
           ]

  kv: [ key KEY . EQ . store VALUE . EOL ]

end
