module Pass_Compose_Func =

  let g (x:string) = ()

  (* Should yield a function string -> unit *)
  let f = () ; g

  let _ = g "a"
