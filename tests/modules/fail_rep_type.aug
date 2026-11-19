module Fail_rep_type =

  (* Only regexps and lenses can be repeated; reporting the type error
     for anything else used to overflow the buffer for the message *)
  let f (x:string) = x
  let y = f*
