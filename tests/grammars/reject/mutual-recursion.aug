grammar mutual-recursion {
  token EOL '\n'
  
  start: 'a' v EOL

  v: w
  w: v
}
