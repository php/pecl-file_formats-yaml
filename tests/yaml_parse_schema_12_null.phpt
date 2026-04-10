--TEST--
Yaml 1.2 Core Schema - null (same as 1.1)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
var_dump(yaml_parse('
a: null
b: Null
c: NULL
d: ~
e:
'));
?>
--EXPECT--
array(5) {
  ["a"]=>
  NULL
  ["b"]=>
  NULL
  ["c"]=>
  NULL
  ["d"]=>
  NULL
  ["e"]=>
  NULL
}
