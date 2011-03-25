(* Parsing yum's config files *)
module Yum =
  autoload xfm

  let eol = Util.del_str "\n"

  let key_re = /[^#;:= \t\n[\/]+/ - /baseurl|gpgkey/

  let eq = del /[ \t]*[:=][ \t]*/ "="
  let secname = /[^]\/]+/

  let value = /[^ \t\n][^\n]*/

  (* We really need to allow comments starting with REM and rem but that     *)
  (* leads to ambiguities with keys 'rem=' and 'REM=' The regular expression *)
  (* to do that cleanly is somewhat annoying to craft by hand; we'd need to  *)
  (* define KEY_RE as /[A-Za-z0-9]+/ - "REM" - "rem"                         *)
  let comment = [ del /([;#].*)?[ \t]*\n/ "\n" ]

  let list_sep = del /([ \t]*(,[ \t]*|\n[ \t]+))|[ \t]+/ "\n\t"
  let list_value = store /[^ \t\n,]+/

  let kv_list(s:string) =
    [ key s . eq . list_value ] .
      [ list_sep . label s . list_value ]* . eol

  let kv = [ key key_re . eq . store value . eol ]

  let sechead = Util.del_str "[" . key secname . Util.del_str "]"
      . del /[ \t]*[;#]?.*/ ""
      . eol

  let entry = comment | kv

  (* A section is a section head, followed by any number of key value    *)
  (* entries, with comments and blank lines freely interspersed. The     *)
  (* continuation lines allowed for baseurl entries make this a little   *)
  (* more interesting: there can be at most one baseurl entry in each    *)
  (* section (more precisely, yum will only obey one of them, but we act *)
  (* as if yum would actually barf)                                      *)
  let section =
    let list_baseurl = kv_list "baseurl" in
      let list_gpgkey = kv_list "gpgkey" in
    [ sechead . (  entry*
                 | entry* . list_baseurl . entry*
                 | entry* . list_gpgkey . entry*
                 | entry* . list_baseurl . entry* . list_gpgkey . entry*
                 | entry* . list_gpgkey . entry* . list_baseurl . entry*)]

  let lns = (comment) * . (section) *

  let filter = (incl "/etc/yum.conf")
      . (incl "/etc/yum.repos.d/*")
      . (incl "/etc/yum/pluginconf.d/*")
      . (excl "/etc/yum/pluginconf.d/versionlock.list")
      . Util.stdexcl

  let xfm = transform lns filter

(* Local Variables: *)
(* mode: caml       *)
(* End:             *)
