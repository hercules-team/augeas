module Test_phpvars =

let conf = "<?php
/**/
/**
 * Multi line comment
 *
 */

/* One line comment */

// Inline comment
global $version;
global $config_version;
$config_version = '1.4.0';

$theme[0]['NAME'] = 'Default';
$theme[0]['PATH'] = SM_PATH . 'themes/default_theme.php';
@include SM_PATH . 'config/config_local.php';
"

test Phpvars.lns get conf =
   { "#comment" }
   { "#comment"
      { "1" = "*" }
      { "2" = "* Multi line comment" }
      { "3" = "*" } }
   {}
   { "#comment"
      { "1" = "One line comment" } }
   {}
   { "#inline" = "Inline comment" }
   { "global" = "version" }
   { "global" = "config_version" }
   { "$config_version" = "'1.4.0'" }
   {}
   { "$theme[0]['NAME']" = "'Default'"}
   { "$theme[0]['PATH']" = "SM_PATH . 'themes/default_theme.php'"}
   { "@include" = "SM_PATH . 'config/config_local.php'"}
