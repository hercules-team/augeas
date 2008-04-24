module Test_sshd =
  let accept_env = "Protocol 2\nAcceptEnv LC_PAPER LC_NAME LC_ADDRESS LC_TELEPHONE LC_MEASUREMENT \nAcceptEnv LC_IDENTIFICATION LC_ALL\n"

  test Sshd.lns get accept_env =
    { "Protocol" = "2" }
    { "AcceptEnv"
        { "1" = "LC_PAPER" }
        { "2" = "LC_NAME" }
        { "3" = "LC_ADDRESS" }
        { "4" = "LC_TELEPHONE" }
        { "5" = "LC_MEASUREMENT" } }
     { "AcceptEnv"
        { "6" = "LC_IDENTIFICATION" }
        { "7" = "LC_ALL" } }

  test Sshd.lns put accept_env after
      rm "AcceptEnv";
      rm "AcceptEnv";
      set "Protocol" "1.5";
      set "X11Forwarding" "yes"
   = "Protocol 1.5\nX11Forwarding yes\n"

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
