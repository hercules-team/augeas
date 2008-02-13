#map
#  grammar pairs
#  include '/pairs.txt' '/pairs'
#end

grammar pairs
  token EOL /([ \t]+.*)?\n/ = '\n'
  token SEP /\s*=\s*/ = '='

  file: counter 'elt' . 
        ([seq 'elt' . store ...] . 
         SEP . 
         [seq 'elt' . store ...] . EOL)*
end
