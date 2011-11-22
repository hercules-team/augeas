module Test_Util =

test Util.empty_c_style get "/* */\n" =
  {  }

test Util.comment_multiline get "/* comment */\n" =
  { "#mcomment"
    { "1" = "comment" }
  }

test Util.comment_multiline get "/*\ncomment\n*/\n" =
  { "#mcomment"
    { "1" = "comment" }
  }

test Util.comment_multiline get "/**
 * Multi line comment
 *
 */\n" =
  { "#mcomment"
    { "1" = "*" }
    { "2" = "* Multi line comment" }
    { "3" = "*" }
  }
