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
include( ROOT . \"/path/to/conf\"	 );
@include SM_PATH . 'config/config_local.php';
class config {
  var $tmppath = \"/tmp\";
  var $offline = 1;
}
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
  { "$theme" = "'Default'"
    { "@arraykey" = "[0]['NAME']" }
    { "#comment" = "end-of line comment"  } }
  { "$theme" = "SM_PATH . 'themes/default_theme.php'"
    { "@arraykey" = "[0][\"PATH\"]" }
  }
  { "$theme" = "'/some//x/path'"
    { "@arraykey" = "[0]['XPATH']" }
  }
  { "define" = "MYVAR"
    { "value" = "ROOT . 'some value'" }
    { "#comment" = "end-of line comment" } }
  { "include_once" = "ROOT . \"/path/to/conf\"" }
  { "include" = "ROOT . \"/path/to/conf\"" }
  { "@include" = "SM_PATH . 'config/config_local.php'" }
  { "config"
    { }
    {"$tmppath" = "\"/tmp\""}
    {"$offline" = "1"}
  }
  { }
