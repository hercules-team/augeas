module Test_IniFile =

  (* ALL TESTS TO RUN *)

  (* entry   :  (a) entry;   (b) entry_nocolon       *)
  (* comment :  (c) comment; (d) comment_nosharp     *)
  (* lns     :  (e) lns;     (f) lns_noempty         *)


  (* TEST a/c/e *)
  let entry_ace  = IniFile.entry "test_ace"
  let record_ace = IniFile.record "record_ace" entry_ace
  let lns_ace    = IniFile.lns record_ace
  let conf_ace   = "# comment with sharp

[section1]
test_ace = value # end of line comment
test_ace =
; comment with colon

"
  test lns_ace get conf_ace = 
      { "#comment" = "comment with sharp" }
      {}
      { "record_ace" = "section1"
          { "test_ace" = "value"
	     { "#comment" = "end of line comment" } }
	  { "test_ace" }
	  { "#comment"  = "comment with colon" }
	  {} }


  (* TEST a/c/f *)
  let entry_acf  = IniFile.entry "test_acf"
  let record_acf = IniFile.record_noempty "record_acf" entry_acf
  let lns_acf    = IniFile.lns_noempty record_acf
  let conf_acf   = "# comment with sharp
[section1]
test_acf = value 
test_acf =
test_acf : value2 # end of line comment
; comment with colon
"
  test lns_acf get conf_acf = 
      { "#comment" = "comment with sharp" }
      { "record_acf" = "section1"
         { "test_acf" = "value" }
	 { "test_acf" }
         { "test_acf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" } }


  (* TEST a/d/e *)
  let comment_ade = IniFile.comment_nosharp
  let entry_ade   = IniFile.entry_setcomment "test_ade" comment_ade
  let record_ade  = IniFile.record_setcomment "record_ade" entry_ade comment_ade
  let lns_ade     = IniFile.lns_setcomment record_ade comment_ade
  let conf_ade    = "; a first comment with colon
[section1]
test_ade = value
test_ade : value2 ; end of line comment
; comment with colon

test_ade =
"
   test lns_ade get conf_ade =
      { "#comment" = "a first comment with colon" }
      { "record_ade" = "section1"
         { "test_ade" = "value" }
         { "test_ade" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 {}
	 { "test_ade" } }


  (* TEST a/d/f *)
  let comment_adf = IniFile.comment_nosharp
  let entry_adf   = IniFile.entry_setcomment "test_adf" comment_adf
  let record_adf  = IniFile.record_noempty_setcomment "record_adf" entry_adf comment_adf
  let lns_adf     = IniFile.lns_noempty_setcomment record_adf comment_adf
  let conf_adf    = "; a first comment with colon
[section1]
test_adf = value
test_adf : value2 ; end of line comment
; comment with colon
test_adf =   
"
   test lns_adf get conf_adf =
      { "#comment" = "a first comment with colon" }
      { "record_adf" = "section1"
         { "test_adf" = "value" }
         { "test_adf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 { "test_adf" } }


  (* TEST b/c/e *)
  let entry_bce  = IniFile.entry_nocolon "test_bce"
  let record_bce = IniFile.record "record_bce" entry_bce
  let lns_bce    = IniFile.lns record_bce
  let conf_bce   = "# comment with sharp

[section1]
test_bce = value # end of line comment
; comment with colon

test_bce =
"
  test lns_bce get conf_bce = 
      { "#comment" = "comment with sharp" }
      {}
      { "record_bce" = "section1"
          { "test_bce" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bce" } }


  (* TEST b/c/f *)
  let entry_bcf  = IniFile.entry_nocolon "test_bcf"
  let record_bcf = IniFile.record_noempty "record_bcf" entry_bcf
  let lns_bcf    = IniFile.lns_noempty record_bcf
  let conf_bcf   = "# comment with sharp
[section1]
test_bcf = value # end of line comment
; comment with colon
test_bcf =
"
  test lns_bcf get conf_bcf = 
      { "#comment" = "comment with sharp" }
      { "record_bcf" = "section1"
          { "test_bcf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bcf" } }


  (* TEST b/d/e *)
  let comment_bde = IniFile.comment_nosharp
  let entry_bde   = IniFile.entry_nocolon_setcomment "test_bde" comment_bde
  let record_bde  = IniFile.record_setcomment "record_bde" entry_bde comment_bde
  let lns_bde     = IniFile.lns_setcomment record_bde comment_bde
  let conf_bde    = "; first comment with colon

[section1]
test_bde = value ; end of line comment
; comment with colon

test_bde =   
"
  test lns_bde get conf_bde = 
      { "#comment" = "first comment with colon" }
      {}
      { "record_bde" = "section1"
          { "test_bde" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bde" } }


  (* TEST b/d/f *)
  let comment_bdf = IniFile.comment_nosharp
  let entry_bdf   = IniFile.entry_nocolon_setcomment "test_bdf" comment_bdf
  let record_bdf  = IniFile.record_noempty_setcomment "record_bdf" entry_bdf comment_bdf
  let lns_bdf     = IniFile.lns_noempty_setcomment record_bdf comment_bdf
  let conf_bdf    = "; first comment with colon
[section1]
test_bdf = value ; end of line comment
; comment with colon
test_bdf = 
"
  test lns_bdf get conf_bdf = 
      { "#comment" = "first comment with colon" }
      { "record_bdf" = "section1"
          { "test_bdf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bdf" } }



