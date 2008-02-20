#map
#  grammar pairs
#  include '/pairs.txt' '/pairs'
#end

grammar pairs
  token EOL /([ \t]+.*)?\n/ = '\n'
  token SEP /\s*=\s*/ = '='
  token WORD /[^ \t\n=]+/ = ''

  file: counter 'elt' . 
        ([seq 'elt' . store WORD] . 
         SEP . 
         [seq 'elt' . store WORD] . EOL)*
end
