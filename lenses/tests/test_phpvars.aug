module Test_phpvars =

let conf = "<?pHp
/**/
/**
 * Multi line comment
 *
 */

/* One line comment */

// Inline comment
# Bash-style comment
global $version;
global $config_version;
$config_version = '1.4.0';
$theme=array();

$theme[0]['NAME'] = 'Default'; // end-of line comment
$theme[0][\"PATH\"] = SM_PATH . 'themes/default_theme.php';
$theme[0]['XPATH'] = '/some//x/path' ;
define ('MYVAR', ROOT . 'some value'); # end-of line comment
include_once( ROOT . \"/path/to/conf\"	 );
@include SM_PATH . 'config/config_local.php';
 ?>
"

test Phpvars.lns get conf =
  {  }
  { "#mcomment"
    { "1" = "*" }
    { "2" = "* Multi line comment" }
    { "3" = "*" }
  }
  {  }
  { "#mcomment"
    { "1" = "One line comment" }
  }
  {  }
  { "#comment" = "Inline comment" }
  { "#comment" = "Bash-style comment" }
  { "global" = "version" }
  { "global" = "config_version" }
  { "$config_version" = "'1.4.0'" }
  { "$theme" = "array()" }
  {  }
  { "$theme[0]['NAME']" = "'Default'"
    { "#comment" = "end-of line comment"  } }
  { "$theme[0][\"PATH\"]" = "SM_PATH . 'themes/default_theme.php'" }
  { "$theme[0]['XPATH']" = "'/some//x/path'" }
  { "define" = "MYVAR"
    { "value" = "ROOT . 'some value'" }
    { "#comment" = "end-of line comment" } }
  { "include_once" = "ROOT . \"/path/to/conf\"" }
  { "@include" = "SM_PATH . 'config/config_local.php'" }
