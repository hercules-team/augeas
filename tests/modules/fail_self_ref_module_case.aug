module Fail_self_ref_module_case =

  (* Like fail_self_ref_module, but with the module name spelled in a
     different case. Module names are case-insensitive, and this used to
     load this file again and again until the stack overflowed *)
  let b = FAIL_SELF_REF_MODULE_CASE.a
  let a = "a"
