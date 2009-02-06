module Test_darkice =

   let conf = "# this is a comment

[general]
duration        = 0
bufferSecs      = 5         # size of internal slip buffer, in seconds

[icecast2-0]
bitrateMode=cbr
format = vorbis
"

   test Darkice.lns get conf =
      { "#comment" = "this is a comment" }
      {}
      { "target" = "general"
         { "duration"  = "0" }
         { "bufferSecs"  = "5"
            { "#comment" = "size of internal slip buffer, in seconds" } }
         {} }
      { "target" = "icecast2-0"
         { "bitrateMode"  = "cbr" }
         { "format"  = "vorbis" } }
