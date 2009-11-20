--TEST--
yaml_parse - syck bug #14384
--CREDITS--
From syck bug #14384
http://pecl.php.net/bugs/bug.php?id=14384
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
	"leading tab"
...
'));
  var_dump(yaml_parse('---
  	"space and tab"
...
'));
var_dump(yaml_parse('---
  "key":	"tab before value"
...
'));
var_dump(yaml_parse('---
  "key":  	"space and tab before value"
...
'));
var_dump(yaml_parse('---
-	"tab before value"
...
'));
var_dump(yaml_parse('---
-  	"space and tab before value"
...
'));
?>
--EXPECTF--
Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 1), context while scanning for the next token (line 2, column 1) in %syaml_parse_006.php on line 5
bool(false)

Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 3), context while scanning for the next token (line 2, column 3) in %syaml_parse_006.php on line 9
bool(false)
array(1) {
  ["key"]=>
  string(16) "tab before value"
}
array(1) {
  ["key"]=>
  string(26) "space and tab before value"
}

Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 2), context while scanning for the next token (line 2, column 2) in %syaml_parse_006.php on line 21
bool(false)

Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 4), context while scanning for the next token (line 2, column 4) in %syaml_parse_006.php on line 25
bool(false)
