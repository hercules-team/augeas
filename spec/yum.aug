# Parsing inistyle files

map
  grammar yum
  include '/etc/yum.conf' '/system/config/yum'
end

grammar yum

  token EOL /\n/ = '\n'
  token INDENT /[ \t]+/ = '\t'
  token EQ /[ \t]*=[ \t]*/ =  '='
  # We really need to allow comments starting with REM and rem but that
  # leads to ambiguities with keys 'rem=' and 'REM=' The regular expression
  # to do that cleanly is somewhat annoying to craft by hand; we'd need to
  # define KEY as /[A-Za-z0-9]+/ - "REM" - "rem"
  token COMMENT /[ \t]*(#.*)?\n/ = '# \n'
  token SECNAME /[A-Za-z0-9]+/ = ''
  token KEY /[A-Za-z0-9_-]+/ = ''
  token VALUE /[^ \t][^\n]*/ = ''

  file: (comment) * . (section) *

  comment: [ COMMENT ]

  section: [ 
             '[' . key SECNAME . ']' . EOL .
             ( comment | kv ) *
           ]

  kv: [ key KEY . EQ . store VALUE . EOL ]

end
