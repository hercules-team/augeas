module Pass_nested_sections =

let word = /[a-zA-Z0-9]+/
let ws = /[ \t]*/
let nl = /\n/

let eol = del (ws . nl) "\n"
let eq = del (ws . "=" . ws) "="
let lbr = del (ws . "{" . ws . nl) " {\n"
let rbr = del (ws . "}" . ws . nl) "}\n"
let indent = del ws ""

let entry = [ indent . key word . eq . store word . eol ]

let rec lns =
  let sec = [ indent . key word . lbr . lns . rbr ] in
  (sec | entry)+


test lns get "key = value\n" = { "key" = "value" }

test lns get "section {
  key1 = v1
  key2 = v2
  section {
    section {
      key4 = v4
    }
  }
  section {
    key5 = v5
  }
}\n" =
  { "section"
    { "key1" = "v1" }
    { "key2" = "v2" }
    { "section" { "section" { "key4" = "v4" } } }
    { "section" { "key5" = "v5" } } }


test lns get "section {
  section {
    key2 = v2
  }
}
section {
  key3 = v3
}
" =
  { "section"
    { "section"
      { "key2" = "v2" } } }
  { "section"
    { "key3" = "v3" } }
