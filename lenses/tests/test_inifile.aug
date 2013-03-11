(*
Module: Test_IniFile
  Provides unit tests and examples for the <IniFile> module.

About: Tests to run

   The tests are run with all combinations of the following
   three parameters:

   > separator   :  (a) default (/[:=]/ "=") ;   (b) "=" "="
   > comment     :  (c) default (/[;#]/ ";") ;   (d) ";" ";"
   > empty lines :  (e) default              ;   (f) noempty

*)

module Test_IniFile =

  (* ALL TESTS TO RUN *)


  (* Group: TEST a/c/e *)
  (* Variable: comment_ace *)
  let comment_ace = IniFile.comment IniFile.comment_re IniFile.comment_default
  (* Variable: sep_ace *)
  let sep_ace     = IniFile.sep IniFile.sep_re IniFile.sep_default
  (* Variable: entry_ace *)
  let entry_ace   = IniFile.entry IniFile.entry_re sep_ace comment_ace
  (* Variable: title_ace *)
  let title_ace   = IniFile.title IniFile.record_re
  (* Variable: record_ace *)
  let record_ace  = IniFile.record title_ace entry_ace
  (* Variable: lns_ace *)
  let lns_ace     = IniFile.lns record_ace comment_ace
  (* Variable: conf_ace *)
  let conf_ace    = "# comment with sharp

[section1]
test_ace = value # end of line comment
test_ace =
test_ace = \"value with spaces\"
; comment with colon

"
  (* Test: lns_ace
      Testing the a/c/e combination *)
  test lns_ace get conf_ace =
      { "#comment" = "comment with sharp" }
      {}
      { "section1"
          { "test_ace" = "value"
	     { "#comment" = "end of line comment" } }
	  { "test_ace" }
          { "test_ace" = "value with spaces" }
	  { "#comment"  = "comment with colon" }
	  {} }

  test lns_ace put conf_ace after
    set "section1/foo" "yes" = "# comment with sharp

[section1]
test_ace = value # end of line comment
test_ace =
test_ace = \"value with spaces\"
; comment with colon

foo=yes
"

  (* Test: lns_ace
       Quotes can appear within bare values *)
  test lns_ace get "[section]\ntest_ace = value \"with quotes\" inside\n" =
  { "section" { "test_ace" = "value \"with quotes\" inside" } }

  (* Group: TEST a/c/f *)
  (* Variable: comment_acf *)
  let comment_acf = IniFile.comment IniFile.comment_re IniFile.comment_default
  (* Variable: sep_acf *)
  let sep_acf     = IniFile.sep IniFile.sep_re IniFile.sep_default
  (* Variable: entry_acf *)
  let entry_acf   = IniFile.entry IniFile.entry_re sep_acf comment_acf
  (* Variable: title_acf *)
  let title_acf   = IniFile.title IniFile.record_re
  (* Variable: record_acf *)
  let record_acf  = IniFile.record_noempty title_acf entry_acf
  (* Variable: lns_acf *)
  let lns_acf     = IniFile.lns_noempty record_acf comment_acf
  (* Variable: conf_acf *)
  let conf_acf   = "# comment with sharp
[section1]
test_acf = value
test_acf =
test_acf : value2 # end of line comment
; comment with colon
"
  (* Test: lns_acf
      Testing the a/c/f combination *)
  test lns_acf get conf_acf =
      { "#comment" = "comment with sharp" }
      { "section1"
         { "test_acf" = "value" }
	 { "test_acf" }
         { "test_acf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" } }


  (* Group: TEST a/d/e *)
  (* Variable: comment_ade *)
  let comment_ade = IniFile.comment ";" ";"
  (* Variable: sep_ade *)
  let sep_ade     = IniFile.sep IniFile.sep_re IniFile.sep_default
  (* Variable: entry_ade *)
  let entry_ade   = IniFile.entry IniFile.entry_re sep_ade comment_ade
  (* Variable: title_ade *)
  let title_ade   = IniFile.title IniFile.record_re
  (* Variable: record_ade *)
  let record_ade  = IniFile.record title_ade entry_ade
  (* Variable: lns_ade *)
  let lns_ade     = IniFile.lns record_ade comment_ade
  (* Variable: conf_ade *)
  let conf_ade    = "; a first comment with colon
[section1]
test_ade = value
test_ade : value2 ; end of line comment
; comment with colon

test_ade =
"
  (* Test: lns_ade
      Testing the a/d/e combination *)
   test lns_ade get conf_ade =
      { "#comment" = "a first comment with colon" }
      { "section1"
         { "test_ade" = "value" }
         { "test_ade" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 {}
	 { "test_ade" } }


  (* Group: TEST a/d/f *)
  (* Variable: comment_adf *)
  let comment_adf = IniFile.comment ";" ";"
  (* Variable: sep_adf *)
  let sep_adf     = IniFile.sep IniFile.sep_re IniFile.sep_default
  (* Variable: entry_adf *)
  let entry_adf   = IniFile.entry IniFile.entry_re sep_adf comment_adf
  (* Variable: title_adf *)
  let title_adf   = IniFile.title IniFile.record_re
  (* Variable: record_adf *)
  let record_adf  = IniFile.record_noempty title_adf entry_adf
  (* Variable: lns_adf *)
  let lns_adf     = IniFile.lns_noempty record_adf comment_adf
  (* Variable: conf_adf *)
  let conf_adf    = "; a first comment with colon
[section1]
test_adf = value
test_adf : value2 ; end of line comment
; comment with colon
test_adf =
"
  (* Test: lns_adf
      Testing the a/d/f combination *)
   test lns_adf get conf_adf =
      { "#comment" = "a first comment with colon" }
      { "section1"
         { "test_adf" = "value" }
         { "test_adf" = "value2"
	    { "#comment" = "end of line comment" } }
	 { "#comment"  = "comment with colon" }
	 { "test_adf" } }


  (* Group: TEST b/c/e *)
  (* Variable: comment_bce *)
  let comment_bce = IniFile.comment IniFile.comment_re IniFile.comment_default
  (* Variable: sep_bce *)
  let sep_bce     = IniFile.sep "=" "="
  (* Variable: entry_bce *)
  let entry_bce   = IniFile.entry IniFile.entry_re sep_bce comment_bce
  (* Variable: title_bce *)
  let title_bce   = IniFile.title IniFile.record_re
  (* Variable: record_bce *)
  let record_bce  = IniFile.record title_bce entry_bce
  (* Variable: lns_bce *)
  let lns_bce     = IniFile.lns record_bce comment_bce
  (* Variable: conf_bce *)
  let conf_bce   = "# comment with sharp

[section1]
test_bce = value # end of line comment
; comment with colon

test_bce =
"
  (* Test: lns_bce
      Testing the b/c/e combination *)
  test lns_bce get conf_bce =
      { "#comment" = "comment with sharp" }
      {}
      { "section1"
          { "test_bce" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bce" } }


  (* Group: TEST b/c/f *)
  (* Variable: comment_bcf *)
  let comment_bcf = IniFile.comment IniFile.comment_re IniFile.comment_default
  (* Variable: sep_bcf *)
  let sep_bcf     = IniFile.sep "=" "="
  (* Variable: entry_bcf *)
  let entry_bcf   = IniFile.entry IniFile.entry_re sep_bcf comment_bcf
  (* Variable: title_bcf *)
  let title_bcf   = IniFile.title IniFile.record_re
  (* Variable: record_bcf *)
  let record_bcf  = IniFile.record_noempty title_bce entry_bcf
  (* Variable: lns_bcf *)
  let lns_bcf     = IniFile.lns_noempty record_bce comment_bcf
  (* Variable: conf_bcf *)
  let conf_bcf   = "# conf with sharp
[section1]
test_bcf = value # end of line comment
; comment with colon
test_bcf =
"
  (* Test: lns_bcf
      Testing the b/c/f combination *)
  test lns_bcf get conf_bcf =
      { "#comment" = "conf with sharp" }
      { "section1"
          { "test_bcf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bcf" } }


  (* Group: TEST b/d/e *)
  (* Variable: comment_bde *)
  let comment_bde = IniFile.comment ";" ";"
  (* Variable: sep_bde *)
  let sep_bde     = IniFile.sep "=" "="
  (* Variable: entry_bde *)
  let entry_bde   = IniFile.entry IniFile.entry_re sep_bde comment_bde
  (* Variable: title_bde *)
  let title_bde   = IniFile.title IniFile.record_re
  (* Variable: record_bde *)
  let record_bde  = IniFile.record title_bde entry_bde
  (* Variable: lns_bde *)
  let lns_bde     = IniFile.lns record_bde comment_bde
  (* Variable: conf_bde *)
  let conf_bde    = "; first comment with colon

[section1]
test_bde = value ; end of line comment
; comment with colon

test_bde =
"
  (* Test: lns_bde
      Testing the b/d/e combination *)
  test lns_bde get conf_bde =
      { "#comment" = "first comment with colon" }
      {}
      { "section1"
          { "test_bde" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  {}
	  { "test_bde" } }


  (* Group: TEST b/d/f *)
  (* Variable: comment_bdf *)
  let comment_bdf = IniFile.comment ";" ";"
  (* Variable: sep_bdf *)
  let sep_bdf     = IniFile.sep "=" "="
  (* Variable: entry_bdf *)
  let entry_bdf   = IniFile.entry IniFile.entry_re sep_bdf comment_bdf
  (* Variable: title_bdf *)
  let title_bdf   = IniFile.title IniFile.record_re
  (* Variable: record_bdf *)
  let record_bdf  = IniFile.record_noempty title_bdf entry_bdf
  (* Variable: lns_bdf *)
  let lns_bdf     = IniFile.lns_noempty record_bdf comment_bdf
  (* Variable: conf_bdf *)
  let conf_bdf    = "; first comment with colon
[section1]
test_bdf = value ; end of line comment
; comment with colon
test_bdf =
"
  (* Test: lns_bdf
      Testing the b/d/f combination *)
  test lns_bdf get conf_bdf =
      { "#comment" = "first comment with colon" }
      { "section1"
          { "test_bdf" = "value"
	     { "#comment" = "end of line comment" } }
	  { "#comment"  = "comment with colon" }
	  { "test_bdf" } }


  (* Group: TEST multiline values *)
  (* Variable: multiline_test *)
  let multiline_test = "test_ace = val1\n  val2\n   val3\n"
  (* Variable: multiline_nl *)
  let multiline_nl = "test_ace =\n  val2\n   val3\n"
  (* Variable: multiline_ace *)
  let multiline_ace = IniFile.entry_multiline IniFile.entry_re sep_ace comment_ace
  (* Test: multiline_ace
       Testing the a/c/e combination with a multiline entry *)
  test multiline_ace get multiline_test =
      { "test_ace" = "val1\n  val2\n   val3" }
  (* Test: multiline_nl
       Multiline values can begin with a single newline *)
  test multiline_ace get multiline_nl =
      { "test_ace" = "\n  val2\n   val3" }

  (* Test: lns_ace
       Ticket #243 *)
  test lns_ace get "[section1]
ticket_243 = \"value1;value2#value3\" # end of line comment
" =
  { "section1"
    { "ticket_243" = "value1;value2#value3"
      { "#comment" = "end of line comment" }
    }
  }

  (* Group: TEST list entries *)
  (* Variable: list_test *)
  let list_test = "test_ace = val1,val2,val3 # a comment\n"
  (* Lens: list_ace *)
  let list_ace = IniFile.entry_list IniFile.entry_re sep_ace RX.word Sep.comma comment_ace
  (* Test: list_ace
       Testing the a/c/e combination with a list entry *)
  test list_ace get list_test =
  { "test_ace"
    { "1" = "val1" }
    { "2" = "val2" }
    { "3" = "val3" }
    { "#comment" = "a comment" }
  }

  (* Variable: list_nocomment_test *)
  let list_nocomment_test = "test_ace = val1,val2,val3 \n"
  (* Lens: list_nocomment_ace *)
  let list_nocomment_ace = IniFile.entry_list_nocomment IniFile.entry_re sep_ace RX.word Sep.comma
  (* Test: list_nocomment_ace
       Testing the a/c/e combination with a list entry without end-of-line comment *)
  test list_nocomment_ace get list_nocomment_test =
  { "test_ace"
    { "1" = "val1" }
    { "2" = "val2" }
    { "3" = "val3" }
  }

  (* Test: IniFile.lns_loose *)
  test IniFile.lns_loose get conf_ace =
  { "section" = ".anon"
    { "#comment" = "comment with sharp" }
    {  }
  }
  { "section" = "section1"
    { "test_ace" = "value"
      { "#comment" = "end of line comment" }
    }
    { "test_ace" }
    { "test_ace" = "value with spaces" }
    { "#comment" = "comment with colon" }
    {  }
  }

  (* Test: IniFile.lns_loose_multiline *)
  test IniFile.lns_loose_multiline get conf_ace =
  { "section" = ".anon"
    { "#comment" = "comment with sharp" }
    {  }
  }
  { "section" = "section1"
    { "test_ace" = "value"
      { "#comment" = "end of line comment" }
    }
    { "test_ace" }
    { "test_ace" = "value with spaces" }
    { "#comment" = "comment with colon" }
    {  }
  }
  
  test IniFile.lns_loose_multiline get multiline_test =
      { "section" = ".anon" { "test_ace" = "val1\n  val2\n   val3" } }

