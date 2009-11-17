--TEST--
Yaml 1.1 Spec - int
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
canonical: 685230
decimal: +685_230
octal: 02472256
hexadecimal: 0x_0A_74_AE
binary: 0b1010_0111_0100_1010_1110
sexagesimal: 190:20:30
'));
?>
--EXPECT--
array(6) {
  ["canonical"]=>
  int(685230)
  ["decimal"]=>
  int(685230)
  ["octal"]=>
  int(685230)
  ["hexadecimal"]=>
  int(685230)
  ["binary"]=>
  int(685230)
  ["sexagesimal"]=>
  int(685230)
}
