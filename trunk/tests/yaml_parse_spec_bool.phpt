--TEST--
Yaml 1.1 Spec - bool
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
canonical: y
answer: NO
logical: True
option: on
'));
?>
--EXPECT--
array(4) {
  ["canonical"]=>
  bool(true)
  ["answer"]=>
  bool(false)
  ["logical"]=>
  bool(true)
  ["option"]=>
  bool(true)
}
