(*
Module: Test_Xml
  Provides unit tests and examples for the <Xml> lens.
*)

module Test_Xml =

(* View: knode
   A simple flag function

   Parameters:
    r:regexp - the pattern for the flag
*)
let knode (r:regexp) = [ key r ]

(************************************************************************
 *                          Group: Utilities lens
 *************************************************************************)
(*
let _ = print_regexp(lens_ctype(Xml.text))
let _ = print_endline ""
*)

(* Group: Comments *)

(* Test: Xml.comment
   Comments get mapped into "#comment" nodes. *)
test Xml.comment get
 "<!-- declarations for <head> & <body> -->" =

  { "#comment" = " declarations for <head> & <body> " }

(* Test: Xml.comment
   This syntax is not understood. *)
test Xml.comment get
 "<!-- B+, B, or B--->" = *

(* Group: Prolog and declarations *)

(* Test: Xml.prolog
   The XML prolog tag is mapped in a "#declaration" node,
   which contains an "#attribute" node with various attributes of the tag. *)
test Xml.prolog get
 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" =

  { "#declaration"
    { "#attribute"
      { "version" = "1.0" }
      { "encoding" = "UTF-8" }
    }
  }

(* Test: Xml.decl_def_item
   !ELEMENT declaration tags are mapped in "!ELEMENT" nodes.
   The associated declaration attribute is mapped in a "#decl" subnode. *)
test Xml.decl_def_item get
 "<!ELEMENT greeting (#PCDATA)>" =

  { "!ELEMENT" = "greeting"
    { "#decl" = "(#PCDATA)" }
  }

(* Test: Xml.decl_def_item
   !ENTITY declaration tags are mapped in "!ENTITY" nodes.
   The associated declaration attribute is mapped in a "#decl" subnode. *)
test Xml.decl_def_item get
 "<!ENTITY da \"&#xD;&#xA;\">" =

  { "!ENTITY" = "da"
    { "#decl" = "&#xD;&#xA;" }
  }

(* Test: Xml.doctype
   !DOCTYPE tags are mapped in "!DOCTYPE" nodes.
   The associated system attribute is mapped in a "SYSTEM" subnode. *)
test Xml.doctype get
 "<!DOCTYPE greeting:foo SYSTEM \"hello.dtd\">" =

  { "!DOCTYPE" = "greeting:foo"
    { "SYSTEM" = "hello.dtd" }
  }

(* Test: Xml.doctype
   This is an example of a !DOCTYPE tag with !ELEMENT children tags. *)
test Xml.doctype get "<!DOCTYPE foo [
<!ELEMENT bar (#PCDATA)>
<!ELEMENT baz (bar)* >
]>" =

  { "!DOCTYPE" = "foo"
    { "!ELEMENT" = "bar"
      { "#decl" = "(#PCDATA)" }
    }
    { "!ELEMENT" = "baz"
      { "#decl" = "(bar)*" }
    }
  }

(* Group: Attributes *)

(* Variable: att_def1 *)
let att_def1 = "<!ATTLIST termdef
id      ID      #REQUIRED
name    CDATA   #IMPLIED>"
(* Variable: att_def2 *)
let att_def2 = "<!ATTLIST list
type    (bullets|ordered|glossary)  \"ordered\">"
(* Variable: att_def3 *)
let att_def3 = "<!ATTLIST form
method  CDATA   #FIXED \"POST\">"

(* Test: Xml.att_list_def *)
test Xml.att_list_def get
 att_def1 =

  { "!ATTLIST" = "termdef"
    { "1"
      { "#name" = "id" }
      { "#type" = "ID" }
      { "#REQUIRED" }
    }
    { "2"
      { "#name" = "name" }
      { "#type" = "CDATA" }
      { "#IMPLIED" }
    }
  }

(* Test: Xml.att_list_def *)
test Xml.att_list_def get
 att_def2 =

  { "!ATTLIST" = "list"
    { "1"
      { "#name" = "type" }
      { "#type" = "(bullets|ordered|glossary)" }
      { "#FIXED" = "ordered" }
    }
  }

(* Test: Xml.att_list_def *)
test Xml.att_list_def get
 att_def3 =

  { "!ATTLIST" = "form"
    { "1"
      { "#name" = "method" }
      { "#type" = "CDATA" }
      { "#FIXED" = "POST" }
    }
  }

(* Test: Xml.notation_def *)
test Xml.notation_def get
 "<!NOTATION not3 SYSTEM \"\">" =

  { "!NOTATION" = "not3"
    { "SYSTEM" = "" }
  }

(* Variable: cdata1 *)
let cdata1 = "<![CDATA[testing]]>"
(* Test: Xml.cdata *)
test Xml.cdata get cdata1 = { "#CDATA" = "testing" }

(* Variable: attr1 *)
let attr1 = " attr1=\"value1\" attr2=\"value2\""
(* Variable: attr2 *)
let attr2 = " attr2=\"foo\""
(* Test: Xml.attributes *)
test Xml.attributes get attr1 =
  { "#attribute"
    { "attr1" = "value1" }
    { "attr2" = "value2" }
  }

(* Test: Xml.attributes *)
test Xml.attributes get " refs=\"A1\nA2  A3\"" =
  { "#attribute"
    { "refs" = "A1\nA2  A3" }
  }

(* Test: Xml.attributes *)
test Xml.attributes put attr1 after rm "/#attribute[1]";
                                    set "/#attribute/attr2" "foo" = attr2

(* test quoting *)
(* well formed values *)
test Xml.attributes get " attr1=\"value1\"" = { "#attribute" { "attr1" = "value1" } }
test Xml.attributes get " attr1='value1'" = { "#attribute" { "attr1" = "value1" } }
test Xml.attributes get " attr1='va\"lue1'" = { "#attribute" { "attr1" = "va\"lue1" } }
test Xml.attributes get " attr1=\"va'lue1\"" = { "#attribute" { "attr1" = "va'lue1" } }

(* illegal as per the XML standard *)
test Xml.attributes get " attr1=\"va\"lue1\"" = *
test Xml.attributes get " attr1='va'lue1'" = *

(* malformed values *)
test Xml.attributes get " attr1=\"value1'" = *
test Xml.attributes get " attr1='value1\"" = *

(* Group: empty *)

(* Variable: empty1 *)
let empty1 = "<a/>"
(* Variable: empty2 *)
let empty2 = "<a foo=\"bar\"/>"
(* Variable: empty3 *)
let empty3 = "<a foo=\"bar\"></a>\n"
(* Variable: empty4 *)
let empty4 = "<a foo=\"bar\" far=\"baz\"/>"
(* Test: Xml.empty_element *)
test Xml.empty_element get empty1 = { "a" = "#empty" }
(* Test: Xml.empty_element *)
test Xml.empty_element get empty2 =
  { "a" = "#empty" { "#attribute" { "foo" = "bar"} } }

(* Test: Xml.empty_element *)
test Xml.empty_element put empty1 after set "/a/#attribute/foo" "bar" = empty2

(* Test: Xml.empty_element
   The attribute node must be the first child of the element *)
test Xml.empty_element put empty1 after set "/a/#attribute/foo" "bar";
                                        set "/a/#attribute/far" "baz" = empty4

(* Test: Xml.content *)
test Xml.content put "<a><b/></a>" after clear "/a/b" = "<a><b></b>\n</a>"


(* Group: Full lens *)

(* Test: Xml.lns *)
test Xml.lns put "<a></a >" after set "/a/#text[1]" "foo";
                                 set "/a/#text[2]" "bar" = "<a>foobar</a >"

(* Test: Xml.lns *)
test Xml.lns get "<?xml version=\"1.0\"?>
<!DOCTYPE catalog PUBLIC \"-//OASIS//DTD XML Catalogs V1.0//EN\"
  \"file:///usr/share/xml/schema/xml-core/catalog.dtd\">
  <doc/>" =
  { "#declaration"
  { "#attribute"
    { "version" = "1.0" }
  }
  }
  { "!DOCTYPE" = "catalog"
    { "PUBLIC"
      { "#literal" = "-//OASIS//DTD XML Catalogs V1.0//EN" }
      { "#literal" = "file:///usr/share/xml/schema/xml-core/catalog.dtd" }
    }
  }
  { "doc" = "#empty" }

(* Test: Xml.lns *)
test Xml.lns get "<oor:component-data xmlns:oor=\"http://openoffice.org/2001/registry\"/>
" =
  { "oor:component-data" = "#empty"
    { "#attribute"
      { "xmlns:oor" = "http://openoffice.org/2001/registry" }
    }
  }

(* Variable: input1 *)
let input1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<html>\r
    <head>
        <title>Wiki</title>
    </head>
    <body>
        <h1>Augeas</h1>
        <p class=\"main\">Augeas is now able to parse XML files!</p>
        <ul>
            <li>Translate from XML to a tree syntax</li>
            <li>Translate from the tree back to XML</li> <!-- this is some comment -->
            <li>this</li>
        </ul>
    </body>
</html>
"

(* Test: Xml.doc
   Test <input1> with <Xml.doc> *)
test Xml.doc get input1 =
  { "#declaration"
    { "#attribute"
      { "version" = "1.0" }
      { "encoding" = "UTF-8" }
    }
  }
  { "html"
    { "#text" = "\r\n    " }
    { "head"
      { "#text" = "\n        " }
      { "title"
        { "#text" = "Wiki" }
      }
      { "#text" = "    " }
    }
    { "#text" = "    " }
    { "body"
      { "#text" = "
        " }
      { "h1"
        { "#text" = "Augeas" }
      }
      { "#text" = "        " }
      { "p"
        { "#attribute"
          { "class" = "main" }
        }
        { "#text" = "Augeas is now able to parse XML files!" }
      }
      { "#text" = "        " }
      { "ul"
        { "#text" = "\n            " }
        { "li"
          { "#text" = "Translate from XML to a tree syntax" }
        }
        { "#text" = "            " }
        { "li"
          { "#text" = "Translate from the tree back to XML" }
        }
        { "#text" = " " }
        { "#comment" = " this is some comment " }
        { "#text" = "
            " }
        { "li"
          { "#text" = "this" }
        }
        { "#text" = "        " }
      }
      { "#text" = "    " }
    }
  }

(* Test: Xml.doc
   Modify <input1> with <Xml.doc> *)
test Xml.doc put input1 after rm "/html/body" =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<html>\r
    <head>
        <title>Wiki</title>
    </head>
    </html>
"


(* Variable: ul1 *)
let ul1 = "
<ul>
 <li>test1</li>
  <li>test2</li>
   <li>test3</li>
    <li>test4</li>
</ul>
"

test Xml.doc get ul1 =
  { "ul"
    { "#text" = "
 " }
    { "li"
      { "#text" = "test1" }
    }
    { "#text" = "  " }
    { "li"
      { "#text" = "test2" }
    }
    { "#text" = "   " }
    { "li"
      { "#text" = "test3" }
    }
    { "#text" = "    " }
    { "li"
      { "#text" = "test4" }
    }
  }


test Xml.doc put ul1 after set "/ul/li[3]/#text" "bidon" = "
<ul>
 <li>test1</li>
  <li>test2</li>
   <li>bidon</li>
    <li>test4</li>
</ul>
"

test Xml.doc put ul1 after rm "/ul/li[2]" = "
<ul>
 <li>test1</li>
     <li>test3</li>
    <li>test4</li>
</ul>
"


(* #text nodes don't move when inserting a node, the result depends on where the node is added *)
test Xml.doc put ul1 after insb "a" "/ul/li[2]" = "
<ul>
 <li>test1</li>
  <a></a>
<li>test2</li>
   <li>test3</li>
    <li>test4</li>
</ul>
"

test Xml.doc put ul1 after insa "a" "/ul/li[1]" = "
<ul>
 <li>test1</li>
<a></a>
  <li>test2</li>
   <li>test3</li>
    <li>test4</li>
</ul>
"

(* Attributes must be added before text nodes *)
test Xml.doc put ul1 after insb "#attribute" "/ul/li[2]/#text";
                           set "/ul/li[2]/#attribute/bidon" "gazou";
                           set "/ul/li[2]/#attribute/foo" "bar" = "
<ul>
 <li>test1</li>
  <li bidon=\"gazou\" foo=\"bar\">test2</li>
   <li>test3</li>
    <li>test4</li>
</ul>
"

(* if empty element is allowed to be as root, this test triggers error *)
test Xml.lns get "<doc>
<a><c/><b><c/></b><c/><c/><a></a></a>
</doc>" =
  { "doc"
    { "#text" = "\n" }
    { "a"
      { "c" = "#empty" }
      { "b"
        { "c" = "#empty" }
      }
      { "c" = "#empty" }
      { "c" = "#empty" }
      { "a" }
    }
  }

let p01pass2 = "<?PI before document element?>
<!-- comment after document element-->
<?PI before document element?>
<!-- comment after document element-->
<?PI before document element?>
<!-- comment after document element-->
<?PI before document element?>
<!DOCTYPE doc
[
<!ELEMENT doc ANY>
<!ELEMENT a ANY>
<!ELEMENT b ANY>
<!ELEMENT c ANY>
]>
<doc>
<a><b><c/></b></a>
</doc>
<!-- comment after document element-->
<?PI after document element?>
<!-- comment after document element-->
<?PI after document element?>
<!-- comment after document element-->
<?PI after document element?>
"

test Xml.lns get p01pass2 =
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "before document element" }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "before document element" }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "before document element" }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "before document element" }
  }
  { "!DOCTYPE" = "doc"
    { "!ELEMENT" = "doc"
      { "#decl" = "ANY" }
    }
    { "!ELEMENT" = "a"
      { "#decl" = "ANY" }
    }
    { "!ELEMENT" = "b"
      { "#decl" = "ANY" }
    }
    { "!ELEMENT" = "c"
      { "#decl" = "ANY" }
    }
  }
  { "doc"
    { "#text" = "
" }
    { "a"
      { "b"
        { "c" = "#empty" }
      }
    }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "after document element" }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "after document element" }
  }
  { "#comment" = " comment after document element" }
  { "#pi"
    { "#target" = "PI" }
    { "#instruction" = "after document element" }
  }


(* various valid Name constructions *)
test Xml.lns get "<doc>\n<A:._-0/>\n<::._-0/>\n<_:._-0/>\n<A/>\n<_/>\n<:/>\n</doc>" =
  { "doc"
    { "#text" = "\n" }
    { "A:._-0" = "#empty" }
    { "::._-0" = "#empty" }
    { "_:._-0" = "#empty" }
    { "A" = "#empty" }
    { "_" = "#empty" }
    { ":" = "#empty" }
  }

test Xml.lns get "<doc>
<abcdefghijklmnopqrstuvwxyz/>
<ABCDEFGHIJKLMNOPQRSTUVWXYZ/>
<A01234567890/>
<A.-:/>
</doc>" =
  { "doc"
    { "#text" = "\n" }
    { "abcdefghijklmnopqrstuvwxyz" = "#empty" }
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" = "#empty" }
    { "A01234567890" = "#empty" }
    { "A.-:" = "#empty" }
  }


let p06fail1 = "<!--non-validating processors may pass this instance because they don't check the IDREFS attribute type-->
<!DOCTYPE doc
[
<!ELEMENT doc (a|refs)*>
<!ELEMENT a EMPTY>
<!ELEMENT refs EMPTY>
<!ATTLIST refs refs IDREFS #REQUIRED>
<!ATTLIST a id ID #REQUIRED>
]>
<doc>
<a id=\"A1\"/><a id=\"A2\"/><a id=\"A3\"/>
<refs refs=\"\"/>
</doc>"

(* we accept this test because we do not verify XML references *)
test Xml.lns get p06fail1 =
  { "#comment" = "non-validating processors may pass this instance because they don't check the IDREFS attribute type" }
  { "!DOCTYPE" = "doc"
    { "!ELEMENT" = "doc"
      { "#decl" = "(a|refs)*" }
    }
    { "!ELEMENT" = "a"
      { "#decl" = "EMPTY" }
    }
    { "!ELEMENT" = "refs"
      { "#decl" = "EMPTY" }
    }
    { "!ATTLIST" = "refs"
      { "1"
        { "#name" = "refs" }
        { "#type" = "IDREFS" }
        { "#REQUIRED" }
      }
    }
    { "!ATTLIST" = "a"
      { "1"
        { "#name" = "id" }
        { "#type" = "ID" }
        { "#REQUIRED" }
      }
    }
  }
  { "doc"
    { "#text" = "
" }
    { "a" = "#empty"
      { "#attribute"
        { "id" = "A1" }
      }
    }
    { "a" = "#empty"
      { "#attribute"
        { "id" = "A2" }
      }
    }
    { "a" = "#empty"
      { "#attribute"
        { "id" = "A3" }
      }
    }
    { "refs" = "#empty"
      { "#attribute"
        { "refs" = "" }
      }
    }
  }

(* we accept dquote, but not single quotes, because of resulting ambiguity *)
let p10pass1_1 = "<doc><A a=\"asdf>'&#34;>\nasdf\n    ?>%\"/></doc>"
let p10pass1_2 = "<doc><A a='\"\">&#39;&#34;'/></doc>"

test Xml.lns get p10pass1_1 =
  { "doc"
    { "A" = "#empty"
      { "#attribute"
        { "a" = "asdf>'&#34;>\nasdf\n    ?>%" }
      }
    }
  }

test Xml.lns get p10pass1_2 =
  { "doc"
    { "A" = "#empty"
      { "#attribute"
        { "a" = "\"\">&#39;&#34;" }
      }
    }
  }

(* here again, test exclude single quote *)
let p11pass1 = "<!--Inability to resolve a notation should not be reported as an error-->
<!DOCTYPE doc
[
<!ELEMENT doc EMPTY>
<!NOTATION not1 SYSTEM \"a%a&b&#0<!ELEMENT<!--<?</>?>/\''\">
<!NOTATION not3 SYSTEM \"\">
]>
<doc></doc>"

test Xml.lns get p11pass1 =
  { "#comment" = "Inability to resolve a notation should not be reported as an error" }
  { "!DOCTYPE" = "doc"
    { "!ELEMENT" = "doc"
      { "#decl" = "EMPTY" }
    }
    { "!NOTATION" = "not1"
      { "SYSTEM" = "a%a&b&#0<!ELEMENT<!--<?</>?>/\''" }
    }
    { "!NOTATION" = "not3"
      { "SYSTEM" = "" }
    }
  }
  { "doc" }

test Xml.lns get "<doc>a%b%&lt;/doc>&#60;/doc>]]&lt;&amp;</doc>" =
  { "doc"
    { "#text" = "a%b%&lt;/doc>&#60;/doc>]]&lt;&amp;" }
  }

let p15pass1 = "<!--a
<!DOCTYPE
<?-
]]>-<[ CDATA [
\"- -'-
-<doc>-->
<!---->
<doc></doc>"

test Xml.lns get p15pass1 =
  { "#comment" = "a
<!DOCTYPE
<?-
]]>-<[ CDATA [
\"- -'-
-<doc>" }
  { "#comment" = "" }
  { "doc" }

let p22pass3 = "<?xml version=\"1.0\"?>
<!--comment--> <?pi some instruction ?>
<doc><?pi?></doc>"

test Xml.lns get p22pass3 =
  { "#declaration"
    { "#attribute"
      { "version" = "1.0" }
    }
  }
  { "#comment" = "comment" }
  { "#pi"
    { "#target" = "pi" }
    { "#instruction" = "some instruction" }
  }
  { "doc"
    { "#pi"
      { "#target" = "pi" }
    }
  }

let p25pass2 = "<?xml version


=


\"1.0\"?>
<doc></doc>"

test Xml.lns get p25pass2 =
  { "#declaration"
    { "#attribute"
      { "version" = "1.0" }
    }
  }
  { "doc" }


test Xml.lns get "<!DOCTYPE

doc

[
<!ELEMENT doc EMPTY>
]>
<doc></doc>" =
  { "!DOCTYPE" = "doc"
    { "!ELEMENT" = "doc"
      { "#decl" = "EMPTY" }
    }
  }
  { "doc" }

test Xml.lns get "<doc></doc  \n>" = { "doc" }

test Xml.lns get "<a><doc att=\"val\" \natt2=\"val2\" att3=\"val3\"/></a>" =
  { "a"
    { "doc" = "#empty"
      { "#attribute"
        { "att" = "val" }
        { "att2" = "val2" }
        { "att3" = "val3" }
      }
    }
  }

test Xml.lns get "<doc/>" = { "doc" = "#empty" }

test Xml.lns get "<a><![CDATA[Thu, 13 Feb 2014 12:22:35 +0000]]></a>" =
  { "a"
    { "#CDATA" = "Thu, 13 Feb 2014 12:22:35 +0000" } }

(* failure tests *)
(* only one document element *)
test Xml.lns get "<doc></doc><bad/>" = *

(* document element must be complete *)
test Xml.lns get "<doc>" = *

(* emtpy document is rejected *)
test Xml.lns get "" = *

(* malformed element *)
test Xml.lns get "<a><A@/></a>" = *

(* a Name cannot start with a digit *)
test Xml.lns get "<a><0A/></a>" = *

(* no space before "CDATA" *)
test Xml.lns get "<doc><![ CDATA[a]]></doc>" = *

(* no space after "CDATA" *)
test Xml.lns get "<doc><![CDATA [a]]></doc>" = *

(* FIXME: CDSect's can't nest *)
test Xml.lns get "<doc>
<![CDATA[
<![CDATA[XML doesn't allow CDATA sections to nest]]>
]]>
</doc>" =
  { "doc"
    { "#text" = "\n" }
    { "#CDATA" = "\n<![CDATA[XML doesn't allow CDATA sections to nest" }
    { "#text" = "\n]]" }
    { "#text" = ">\n" } }

(* Comment is illegal in VersionInfo *)
test Xml.lns get "<?xml version <!--bad comment--> =\"1.0\"?>
<doc></doc>" = *

(* only declarations in DTD *)
test Xml.lns get "<!DOCTYPE doc [
<!ELEMENT doc EMPTY>
<doc></doc>
]>" = *

(* we do not support external entities *)
test Xml.lns get "<!DOCTYPE doc [
<!ENTITY % eldecl \"<!ELEMENT doc EMPTY>\">
%eldecl;
]>
<doc></doc>" = *

(* Escape character in attributes *)
test Xml.lns get "<a password=\"my\!pass\" />" =
  { "a" = "#empty"
    { "#attribute" { "password" = "my\!pass" } } }
