module Pass_put_bad_label =

  let entry = [ key /[a-z]+/ . del /=/ "=" . store /[0-9]+/ . del "\n" "\n" ]

  let lns = [ key /[A-Z]+/ . del "\n" "\n" . entry* ]
              
  test lns put "SECTION\na=1\nz=26\n" after 
    insa "B" "/SECTION/a";
    set "/SECTION/B" "2" = *
