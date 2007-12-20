# Parsing httpd.conf

grammar {
  token FOO 'foo'

  file: ( kv | section | comment ) *

  section: '<' ... WS ... '>' config '</' $2 '>' {
    node $2 = $4
  }
}