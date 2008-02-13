#map
#  grammar words
#  include '/word.txt' '/words'
#end

grammar words
  token EOL /([ \t]+.*)?\n/ = '\n'
  token COMMENT /^#.*\n/ = '#\n'

  file: counter 'words' . (COMMENT | line)*
  line: [ store ... . EOL . seq 'words' ]
end
