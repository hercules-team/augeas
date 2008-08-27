module Fail_del_default_check =

  (* Not valid since the default value "NO" does not match /[a-z]+/ *)
  let lns = del /[a-z]+/ "NO"
