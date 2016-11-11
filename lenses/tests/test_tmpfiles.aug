(*
Module: Test_Tmpfiles
  Provides unit tests and examples for the <Tmpfiles> lens.
*)

module Test_Tmpfiles =

(************************************************************************
 * Group:                 VALID EXAMPLES
 *************************************************************************)
  (* Variable: simple
One line, simple example *)
  let simple = "d /run/user 0755 root mysql 10d -\n"

  (* Variable: simple_tree
Tree for <simple> *)
  let simple_tree =
    {
        "1"
        { "type" = "d" }
        { "path" = "/run/user" }
        { "mode" = "0755" }
        { "uid" = "root" }
        { "gid" = "mysql" }
        { "age" = "10d" }
        { "argument" = "-" }
    }

  (* Variable: complex
A more complex example, comes from the manual *)
  let complex = "#Type Path        Mode UID  GID  Age Argument\nd    /run/user   0755 root root 10d -\nL    /tmp/foobar -    -    -    -   /dev/null\n"

  (* Variable: complex_tree
Tree for <complex> and <trailing_ws> *)
  let complex_tree =
      { "#comment" = "Type Path        Mode UID  GID  Age Argument" }
      { "1"
        { "type" = "d" }
        { "path" = "/run/user" }
        { "mode" = "0755" }
        { "uid" = "root" }
        { "gid" = "root" }
        { "age" = "10d" }
        { "argument" = "-" }
      }
      { "2"
        { "type" = "L" }
        { "path" = "/tmp/foobar" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "-" }
        { "age" = "-" }
        { "argument" = "/dev/null" }
      }

  (* Variable: trailing_ws
The complex example with extra spaces *)
  let trailing_ws = "  #Type Path        Mode UID  GID  Age Argument  \n    d    /run/user   0755 root root 10d -   \t\n    L    /tmp/foobar -    -    -    -   /dev/null\t\n"

  (* Variable: empty
Empty example *)
  let empty = "\n\n\n"

  (* Variable: exclamation_mark
Example with an exclamation mark in the type *)
  let exclamation_mark = "D! /tmp/foo - - - - -\n"

  (* Variable: exclamation_mark_tree
Tree for <exclamation_mark> *)
  let exclamation_mark_tree =
    {
        "1"
        { "type" = "D!" }
        { "path" = "/tmp/foo" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "-" }
        { "age" = "-" }
        { "argument" = "-" }
    }

  (* Variable: short
Example with only type and path *)
  let short = "A+ /tmp/foo\n"

  (* Variable: short_tree
Tree for <short> *)
  let short_tree =
    {
        "1"
        { "type" = "A+" }
        { "path" = "/tmp/foo" }
    }

  (* Variable: short_mode
Example with only 3 fields *)
  let short_mode = "c+! /tmp/foo ~0755\n"

  (* Variable: short_mode_tree
Tree for <short_mode> *)
  let short_mode_tree =
    {
        "1"
        { "type" = "c+!" }
        { "path" = "/tmp/foo" }
        { "mode" = "~0755" }
    }

  (* Variable: short_uid
Example with only 4 fields *)
  let short_uid = "A+ /tmp/foo   -   0\n"

  (* Variable: short_uid_tree
Tree for <short_uid> *)
  let short_uid_tree =
    {
        "1"
        { "type" = "A+" }
        { "path" = "/tmp/foo" }
        { "mode" = "-" }
        { "uid" = "0" }
    }

  (* Variable: short_gid
Example with only 5 fields *)
  let short_gid = "z /tmp/bar/foo -\t- augd\n"

  (* Variable: short_gid_tree
Tree for <short_gid> *)
  let short_gid_tree =
    {
        "1"
        { "type" = "z" }
        { "path" = "/tmp/bar/foo" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "augd" }
    }

  (* Variable: short_age
Example with only 6 fields *)
  let short_age = "H /var/tmp/fooBarFOO - jj jj ~10d\n"

  (* Variable: short_age_tree
Tree for <short_age> *)
  let short_age_tree =
    {
        "1"
        { "type" = "H" }
        { "path" = "/var/tmp/fooBarFOO" }
        { "mode" = "-" }
        { "uid" = "jj" }
        { "gid" = "jj" }
        { "age" = "~10d" }
    }

  (* Variable: complex_arg
Complex argument example. That one comes from the manual *)
  let complex_arg = "t /run/screen - - - - user.name=\"John Smith\" security.SMACK64=screen\n"

  (* Variable: complex_arg_tree
Tree for <complex_arg> *)
  let complex_arg_tree =
    {
        "1"
        { "type" = "t" }
        { "path" = "/run/screen" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "-" }
        { "age" = "-" }
        { "argument" = "user.name=\"John Smith\" security.SMACK64=screen" }
    }

  (* Variable: valid_short_args
A short argument value example. *)
  let valid_short_args = "h /var/log/journal - - - - C\nh /var/log/journal - - - - +C\n"

  (* Variable: valid_short_args_tree
Tree for <valid_short_args> *)
  let valid_short_args_tree =
    {
        "1"
        { "type" = "h" }
        { "path" = "/var/log/journal" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "-" }
        { "age" = "-" }
        { "argument" = "C" }
    }
    {
        "2"
        { "type" = "h" }
        { "path" = "/var/log/journal" }
        { "mode" = "-" }
        { "uid" = "-" }
        { "gid" = "-" }
        { "age" = "-" }
        { "argument" = "+C" }
    }

  (* Variable: valid_age
Example with a complex age. *)
  let valid_age = "v /var/tmp/js 4221 johnsmith - ~10d12h\n"

  (* Variable: valid_age_tree
Tree for <valid_age> *)
  let valid_age_tree =
    {
        "1"
        { "type" = "v" }
        { "path" = "/var/tmp/js" }
        { "mode" = "4221" }
        { "uid" = "johnsmith" }
        { "gid" = "-" }
        { "age" = "~10d12h" }
    }

  (* Variable: valid_second
Example with full age unit *)
  let valid_second = "p+ /var/tmp - jsmith - 0second\n"

  (* Variable: valid_second_tree
Tree for <valid_second> *)
  let valid_second_tree =
    {
        "1"
        { "type" = "p+" }
        { "path" = "/var/tmp" }
        { "mode" = "-" }
        { "uid" = "jsmith" }
        { "gid" = "-" }
        { "age" = "0second" }
    }

  (* Variable: valid_days
Example with full age unit (plural) *)
  let valid_days = "x /var/tmp/manu - jonhsmith - 9days\n"

  (* Variable: valid_days_tree
Tree for <valid_days> *)
  let valid_days_tree =
    {
        "1"
        { "type" = "x" }
        { "path" = "/var/tmp/manu" }
        { "mode" = "-" }
        { "uid" = "jonhsmith" }
        { "gid" = "-" }
        { "age" = "9days" }
    }

  (* Variable: percent
Test with a percent sign *)
  let percent = "m /var/log/%m 2755 root systemdjournal - -\n"

  (* Variable: percent_tree
Tree for <percent> *)
  let percent_tree =
    {
        "1"
        { "type" = "m" }
        { "path" = "/var/log/%m" }
        { "mode" = "2755" }
        { "uid" = "root" }
        { "gid" = "systemdjournal" }
        { "age" = "-" }
        { "argument" = "-" }
    }

  (* Variable: hyphen
Test with a hyphen in gid *)
  let hyphen = "L /var/log/journal 2755 root systemd-journal - -\n"

  (* Variable: hyphen_tree
Tree for <hyphen> *)
  let hyphen_tree =
    {
        "1"
        { "type" = "L" }
        { "path" = "/var/log/journal" }
        { "mode" = "2755" }
        { "uid" = "root" }
        { "gid" = "systemd-journal" }
        { "age" = "-" }
        { "argument" = "-" }
    }

  (* Variable: valid_base
A valid test to be re-used by the failure cases *)
  let valid_base = "H /var/tmp/js 0000 jonhsmith 60 1s foo\n"

  (* Variable: valid_base_tree
Tree for <valid_base> *)
  let valid_base_tree =
    {
        "1"
        { "type" = "H" }
        { "path" = "/var/tmp/js" }
        { "mode" = "0000" }
        { "uid" = "jonhsmith" }
        { "gid" = "60" }
        { "age" = "1s" }
        { "argument" = "foo" }
    }

  (* Variable: mode3
Mode field example with only three digits *)
  let mode3 = "c+! /tmp/foo 755\n"

  (* Variable: mode3_tree
Tree for <mode3> *)
  let mode3_tree =
    {
        "1"
        { "type" = "c+!" }
        { "path" = "/tmp/foo" }
        { "mode" = "755" }
    }

(************************************************************************
 * Group:                 INVALID EXAMPLES
 *************************************************************************)

  (* Variable: invalid_too_short
Invalid example that do not contain path *)
  let invalid_too_short = "H\n"

  (* Variable: invalid_age
Invalid example that contain invalid age  *)
  let invalid_age = "H /var/tmp/js 0000 jonhsmith 60 1sss foo\n"

  (* Variable: invalid_type
Invalid example that contain invalid type (bad letter) *)
  let invalid_type = "e /var/tmp/js 0000 jonhsmith 60 1s foo\n"

  (* Variable: invalid_type_num
 Invalid example that contain invalid type (numeric) *)
  let invalid_type_num = "1 /var/tmp/js 0000 jonhsmith 60 1s foo\n"

  (* Variable: invalid_mode
Invalid example that contain invalid mode (bad int) *)
  let invalid_mode = "H /var/tmp/js 8000 jonhsmith 60 1s foo\n"

  (* Variable: invalid_mode_alpha
Invalid example that contain invalid mode (letter) *)
  let invalid_mode_alpha = "H /var/tmp/js a000 jonhsmith 60 1s foo\n"

  test Tmpfiles.lns get simple = simple_tree

  test Tmpfiles.lns get complex = complex_tree

  test Tmpfiles.lns get trailing_ws = complex_tree

  test Tmpfiles.lns get empty = {}{}{}

  test Tmpfiles.lns get exclamation_mark = exclamation_mark_tree

  test Tmpfiles.lns get short = short_tree

  test Tmpfiles.lns get short_mode = short_mode_tree

  test Tmpfiles.lns get short_uid = short_uid_tree

  test Tmpfiles.lns get short_gid = short_gid_tree

  test Tmpfiles.lns get short_age = short_age_tree

  test Tmpfiles.lns get complex_arg = complex_arg_tree

  test Tmpfiles.lns get valid_short_args = valid_short_args_tree

  test Tmpfiles.lns get valid_second = valid_second_tree

  test Tmpfiles.lns get valid_days = valid_days_tree

  test Tmpfiles.lns get valid_age = valid_age_tree

  test Tmpfiles.lns get percent = percent_tree

  test Tmpfiles.lns get hyphen = hyphen_tree

  test Tmpfiles.lns get valid_base = valid_base_tree

  test Tmpfiles.lns get mode3 = mode3_tree


(* failure cases *)

  test Tmpfiles.lns get invalid_too_short = *

  test Tmpfiles.lns get invalid_age = *

  test Tmpfiles.lns get invalid_type = *

  test Tmpfiles.lns get invalid_type_num = *

  test Tmpfiles.lns get invalid_mode = *

  test Tmpfiles.lns get invalid_mode_alpha = *

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
