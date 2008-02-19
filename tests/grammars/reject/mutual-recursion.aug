grammar mutual-recursion
  token EOL '\n'
  
  start: 'a' . v . EOL

  v: 'v' . w
  w: 'w' . v
end
