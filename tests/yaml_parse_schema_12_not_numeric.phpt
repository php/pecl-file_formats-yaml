--TEST--
Yaml 1.2 Core Schema - values that are NOT numeric
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
// Ensure these edge cases are correctly treated as strings
var_dump(yaml_parse('
bare_dot: .
dot_e: .e5
bare_plus: +
bare_minus: "-"
'));
?>
--EXPECT--
array(4) {
  ["bare_dot"]=>
  string(1) "."
  ["dot_e"]=>
  string(3) ".e5"
  ["bare_plus"]=>
  string(1) "+"
  ["bare_minus"]=>
  string(1) "-"
}
