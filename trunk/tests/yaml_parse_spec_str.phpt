--TEST--
Yaml 1.1 Spec - str
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
string: abcd
'));
?>
--EXPECT--
array(1) {
  ["string"]=>
  string(4) "abcd"
}
