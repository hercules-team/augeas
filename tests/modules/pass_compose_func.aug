module Pass_Compose_Func =

  (* string -> regexp *)
  let f (x:string) = x . /[a-z]/

  (* regexp -> lens *)
  let g (x:regexp) = key x

  (* string -> lens *)
  let h = f ; g

  let _ = h "a"
