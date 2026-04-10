--TEST--
Yaml 1.2 Core Schema - int (0o octal, no binary, no sexagesimal, no underscores)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
var_dump(yaml_parse('
# Valid 1.2 integers
decimal: 685230
negative: -42
hex: 0xFF
octal_12: 0o777
zero: 0
# NOT integers in 1.2 (they were in 1.1)
octal_11: 02472256
binary: 0b1010
sexagesimal: 190:20:30
underscored: 1_000_000
'));
?>
--EXPECT--
array(9) {
  ["decimal"]=>
  int(685230)
  ["negative"]=>
  int(-42)
  ["hex"]=>
  int(255)
  ["octal_12"]=>
  int(511)
  ["zero"]=>
  int(0)
  ["octal_11"]=>
  string(8) "02472256"
  ["binary"]=>
  string(6) "0b1010"
  ["sexagesimal"]=>
  string(9) "190:20:30"
  ["underscored"]=>
  string(9) "1_000_000"
}
