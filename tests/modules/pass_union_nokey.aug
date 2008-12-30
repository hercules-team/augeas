module Pass_union_nokey =

let lns = [ key /[a-z]+/ . store /[0-9]+/
          | [ key /[a-z]+/ ] . del /[ ]+/ " " ]
