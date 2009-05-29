module Test_IniFile =

  (* ALL TESTS TO RUN *)

  (* separator   :  (a) default (/[:=]/ "=") ;   (b) "=" "="     *)
  (* comment     :  (c) default (/[;#]/ ";") ;   (d) ";" ";"     *)
  (* empty lines :  (e) default              ;   (f) noempty     *)


  (* TEST a/c/e *)
  let comment_ace = IniFile.comment IniFile.comment_re IniFile.comment_default
  let sep_ace     = IniFile.sep IniFile.sep_re IniFile.sep_default
  let entry_ace   = IniFile.entry IniFile.entry_re sep_ace comment_ace
  let title_ace   = IniFile.title IniFile.record_re
  let record_ace  = IniFile.record title_ace entry_ace
  let lns_ace     = IniFile.lns record_ace comment_ace
  let conf_ace    = "# comment with sharp

[section1]
test_ace = value # end of line comment
test_ace =
; comment with colon

"
  test lns_ace get conf_ace =
      { "#comment" = "comment with sharp" }
      {}
      { "section1"
          { "test_ace" = "value"
	     { "#comment" = "end of line comment" } }
	  { "test_ace" }
	  { "#comment"  = "comment with colon" }
	  {} }


  (* TEST a/c/f *)
  let comment_acf = IniFile.comment IniFile.comment_re IniFile.comment_default
  let sep_acf     = IniFile.sep IniFile.sep_re IniFile.sep_default
  let entry_acf   = IniFile.entry IniFile.entry_re sep_acf comment_acf
  let title_acf   = IniFile.title IniFile.record_re
  let record_acf  = IniFile.record_noempty title_acf entry_acf
  let lns_acf     = IniFile.lns_noempty record_acf comment_acf
  let conf_acf   = "# comment with sharp
[section1]
test_acf = value
test_acf =
test_acf : value2 # end of line comment
; comment with colon
"
  test lns_acf get conf_acf =
      { "#comment" = "comment with sharp" }
      { "section1"
         { "test_acf" = "value" }
	 { "test_acf" }
         { "test_acf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" } }


  (* TEST a/d/e *)
  let comment_ade = IniFile.comment ";" ";"
  let sep_ade     = IniFile.sep IniFile.sep_re IniFile.sep_default
  let entry_ade   = IniFile.entry IniFile.entry_re sep_ade comment_ade
  let title_ade   = IniFile.title IniFile.record_re
  let record_ade  = IniFile.record title_ade entry_ade
  let lns_ade     = IniFile.lns record_ade comment_ade
  let conf_ade    = "; a first comment with colon
[section1]
test_ade = value
test_ade : value2 ; end of line comment
; comment with colon

test_ade =
"
   test lns_ade get conf_ade =
      { "#comment" = "a first comment with colon" }
      { "section1"
         { "test_ade" = "value" }
         { "test_ade" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 {}
	 { "test_ade" } }


  (* TEST a/d/f *)
  let comment_adf = IniFile.comment ";" ";"
  let sep_adf     = IniFile.sep IniFile.sep_re IniFile.sep_default
  let entry_adf   = IniFile.entry IniFile.entry_re sep_adf comment_adf
  let title_adf   = IniFile.title IniFile.record_re
  let record_adf  = IniFile.record_noempty title_adf entry_adf
  let lns_adf     = IniFile.lns_noempty record_adf comment_adf
  let conf_adf    = "; a first comment with colon
[section1]
test_adf = value
test_adf : value2 ; end of line comment
; comment with colon
test_adf =
"
   test lns_adf get conf_adf =
      { "#comment" = "a first comment with colon" }
      { "section1"
         { "test_adf" = "value" }
         { "test_adf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 { "test_adf" } }


  (* TEST b/c/e *)
  let comment_bce = IniFile.comment IniFile.comment_re IniFile.comment_default
  let sep_bce     = IniFile.sep "=" "="
  let entry_bce   = IniFile.entry IniFile.entry_re sep_bce comment_bce
  let title_bce   = IniFile.title IniFile.record_re
  let record_bce  = IniFile.record title_bce entry_bce
  let lns_bce     = IniFile.lns record_bce comment_bce
  let conf_bce   = "# comment with sharp

[section1]
test_bce = value # end of line comment
; comment with colon

test_bce =
"
  test lns_bce get conf_bce =
      { "#comment" = "comment with sharp" }
      {}
      { "section1"
          { "test_bce" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bce" } }


  (* TEST b/c/f *)
  let comment_bcf = IniFile.comment IniFile.comment_re IniFile.comment_default
  let sep_bcf     = IniFile.sep "=" "="
  let entry_bcf   = IniFile.entry IniFile.entry_re sep_bcf comment_bcf
  let title_bcf   = IniFile.title IniFile.record_re
  let record_bcf  = IniFile.record_noempty title_bce entry_bcf
  let lns_bcf     = IniFile.lns_noempty record_bce comment_bcf
  let conf_bcf   = "# comment with sharp
[section1]
test_bcf = value # end of line comment
; comment with colon
test_bcf =
"
  test lns_bcf get conf_bcf =
      { "#comment" = "comment with sharp" }
      { "section1"
          { "test_bcf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bcf" } }


  (* TEST b/d/e *)
  let comment_bde = IniFile.comment ";" ";"
  let sep_bde     = IniFile.sep "=" "="
  let entry_bde   = IniFile.entry IniFile.entry_re sep_bde comment_bde
  let title_bde   = IniFile.title IniFile.record_re
  let record_bde  = IniFile.record title_bde entry_bde
  let lns_bde     = IniFile.lns record_bde comment_bde
  let conf_bde    = "; first comment with colon

[section1]
test_bde = value ; end of line comment
; comment with colon

test_bde =
"
  test lns_bde get conf_bde =
      { "#comment" = "first comment with colon" }
      {}
      { "section1"
          { "test_bde" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bde" } }


  (* TEST b/d/f *)
  let comment_bdf = IniFile.comment ";" ";"
  let sep_bdf     = IniFile.sep "=" "="
  let entry_bdf   = IniFile.entry IniFile.entry_re sep_bdf comment_bdf
  let title_bdf   = IniFile.title IniFile.record_re
  let record_bdf  = IniFile.record_noempty title_bdf entry_bdf
  let lns_bdf     = IniFile.lns_noempty record_bdf comment_bdf
  let conf_bdf    = "; first comment with colon
[section1]
test_bdf = value ; end of line comment
; comment with colon
test_bdf =
"
  test lns_bdf get conf_bdf =
      { "#comment" = "first comment with colon" }
      { "section1"
          { "test_bdf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bdf" } }



