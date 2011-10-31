module Krb5 =

autoload xfm

let comment = Inifile.comment "#" "#"
let empty = Inifile.empty
let eol = Inifile.eol
let dels = Util.del_str

let indent = del /[ \t]*/ ""
let eq = del /[ \t]*=[ \t]*/ " = "
let eq_openbr = del /[ \t]*=[ \t\n]*\{([ \t]*\n)*/ " = {"
let closebr = del /[ \t]*\}/ "}"

(* These two regexps for realms and apps are not entirely true
   - strictly speaking, there's no requirement that a realm is all upper case
   and an application only uses lowercase. But it's what's used in practice.

   Without that distinction we couldn't distinguish between applications
   and realms in the [appdefaults] section.
*)

let realm_re = /[A-Z][.a-zA-Z0-9-]*/
let app_re = /[a-z][a-zA-Z0-9_]*/
let name_re = /[.a-zA-Z0-9_-]+/

let value = store /[^;# \t\n{}]+/
let entry (kw:regexp) (sep:lens) (comment:lens)
    = [ indent . key kw . sep . value . (comment|eol) ] | comment

let simple_section (n:string) (k:regexp) =
  let title = Inifile.indented_title n in
  let entry = entry k eq comment in
    Inifile.record title entry

let record (t:string) (e:lens) =
  let title = Inifile.indented_title t in
    Inifile.record title e

let libdefaults =
  let option = entry (name_re - "v4_name_convert") eq comment in
  let subsec = [ indent . key /host|plain/ . eq_openbr .
                   (entry name_re eq comment)* . closebr . eol ] in
  let v4_name_convert = [ indent . key "v4_name_convert" . eq_openbr .
                          subsec* . closebr . eol ] in
  record "libdefaults" (option|v4_name_convert)

let login =
  let keys = /krb[45]_get_tickets|krb4_convert|krb_run_aklog/
    |/aklog_path|accept_passwd/ in
    simple_section "login" keys

let appdefaults =
  let option = entry (name_re - "realm" - "application") eq comment in
  let realm = [ indent . label "realm" . store realm_re .
                  eq_openbr . option* . closebr . eol ] in
  let app = [ indent . label "application" . store app_re .
                eq_openbr . (realm|option)* . closebr . eol] in
    record "appdefaults" (option|realm|app)

let realms =
  let simple_option = /kdc|admin_server|database_module|default_domain/
      |/v4_realm|auth_to_local(_names)?|master_kdc|kpasswd_server/
      |/admin_server/ in
  let subsec_option = /v4_instance_convert/ in
  let option = entry simple_option eq comment in
  let subsec = [ indent . key subsec_option . eq_openbr .
                   (entry name_re eq comment)* . closebr . eol ] in
  let realm = [ indent . label "realm" . store realm_re .
                  eq_openbr . (option|subsec)* . closebr . eol ] in
    record "realms" (realm|comment)

let domain_realm =
  simple_section "domain_realm" name_re

let logging =
  let keys = /kdc|admin_server|default/ in
  let xchg (m:regexp) (d:string) (l:string) =
    del m d . label l in
  let xchgs (m:string) (l:string) = xchg m m l in
  let dest =
    [ xchg /FILE[=:]/ "FILE=" "file" . value ]
    |[ xchgs "STDERR" "stderr" ]
    |[ xchgs "CONSOLE" "console" ]
    |[ xchgs "DEVICE=" "device" . value ]
    |[ xchgs "SYSLOG" "syslog" .
         ([ xchgs ":" "severity" . store /[A-Za-z0-9]+/ ].
          [ xchgs ":" "facility" . store /[A-Za-z0-9]+/ ]?)? ] in
  let entry = [ indent . key keys . eq . dest . (comment|eol) ] | comment in
    record "logging" entry

let capaths =
  let realm = [ indent . key realm_re .
                  eq_openbr .
                  (entry realm_re eq comment)* . closebr . eol ] in
    record "capaths" (realm|comment)

let dbdefaults =
  let keys = /database_module|ldap_kerberos_container_dn|ldap_kdc_dn/
    |/ldap_kadmind_dn|ldap_service_password_file|ldap_servers/
    |/ldap_conns_per_server/ in
    simple_section "dbdefaults" keys

let dbmodules =
  let keys = /db_library|ldap_kerberos_container_dn|ldap_kdc_dn/
    |/ldap_kadmind_dn|ldap_service_password_file|ldap_servers/
    |/ldap_conns_per_server/ in
    simple_section "dbmodules" keys

(* This section is not documented in the krb5.conf manpage,
   but the Fermi example uses it. *)
let instance_mapping =
  let value = dels "\"" . store /[^;# \t\n{}]*/ . dels "\"" in
  let map_node = label "mapping" . store /[a-zA-Z0-9\/*]+/ in
  let mapping = [ indent . map_node . eq .
                    [ label "value" . value ] . (comment|eol) ] in
  let instance = [ indent . key name_re .
                     eq_openbr . (mapping|comment)* . closebr . eol ] in
    record "instancemapping" instance

let kdc =
  simple_section "kdc" /profile/

let pam =
  simple_section "pam" name_re

let lns = (comment|empty)* .
  (libdefaults|login|appdefaults|realms|domain_realm
  |logging|capaths|dbdefaults|dbmodules|instance_mapping|kdc|pam)*

let xfm = transform lns (incl "/etc/krb5.conf")
