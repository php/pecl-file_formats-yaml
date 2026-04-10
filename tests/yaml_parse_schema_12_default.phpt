--TEST--
Yaml schema default (0) behaves as 1.1
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
// Verify default schema (0) gives YAML 1.1 behavior
var_dump(ini_get('yaml.decode_schema'));
var_dump(yaml_parse('
bool_yes: yes
bool_no: NO
octal: 02472256
'));
?>
--EXPECT--
string(1) "0"
array(3) {
  ["bool_yes"]=>
  bool(true)
  ["bool_no"]=>
  bool(false)
  ["octal"]=>
  int(685230)
}
