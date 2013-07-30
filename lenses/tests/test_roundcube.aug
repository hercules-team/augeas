module Test_roundcube =

let conf = "<?phP

/*
+-----------------------------------------------------------------------+
| Multiline comment                                                     |
+-----------------------------------------------------------------------+

*/

// This is a single line comment

// The following is an empty comment
//
$rcmail_config = array();

$rcmail_config['debug_level'] = 1;

$rcmail_config['log_driver'] = 'file';

$rcmail_config['log_date_format'] = 'd-M-Y H:i:s O'; // The quick brown fox

$rcmail_config['default_imap_folders'] = array('INBOX', 'Drafts', 'Sent', 'Junk', 'Trash'); # Jumps over the lazy dog
"

test Roundcube.lns get conf =
   { }
   { "#mcomment"
    { "1" = "+-----------------------------------------------------------------------+" }
    { "2" = "| Multiline comment                                                     |" }
    { "3" = "+-----------------------------------------------------------------------+" }
   }
   { }
   { "#comment" = "This is a single line comment" }
   { }
   { "#comment" = "The following is an empty comment" }
   { }
   { "$rcmail_config" = "array()" }
   { }
   { "debug_level" = "1" }
   { }
   { "log_driver" = "'file'" }
   { }
   { "log_date_format" = "'d-M-Y H:i:s O'"
      { "#comment" = "The quick brown fox" }
   }
   { }
   { "default_imap_folders" = "array('INBOX', 'Drafts', 'Sent', 'Junk', 'Trash')"
      { "#comment" = "Jumps over the lazy dog" }
   }
   
