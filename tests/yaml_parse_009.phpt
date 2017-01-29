--TEST--
yaml_parse - bool treatment
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_bool=1
--FILE--
<?php
$data = <<<YAML
canonical: y
answer: NO
logical: True
option: on
abbreviation: N
YAML;

var_dump(yaml_parse($data));

ini_set('yaml.decode_bool', 0);
var_dump(yaml_parse($data));

ini_set('yaml.decode_bool', 2);
var_dump(yaml_parse($data));
?>
--EXPECT--
array(5) {
  ["canonical"]=>
  bool(true)
  ["answer"]=>
  bool(false)
  ["logical"]=>
  bool(true)
  ["option"]=>
  bool(true)
  ["abbreviation"]=>
  bool(false)
}
array(5) {
  ["canonical"]=>
  string(1) "y"
  ["answer"]=>
  string(2) "NO"
  ["logical"]=>
  string(4) "True"
  ["option"]=>
  string(2) "on"
  ["abbreviation"]=>
  string(1) "N"
}
array(5) {
  ["canonical"]=>
  string(1) "y"
  ["answer"]=>
  bool(false)
  ["logical"]=>
  bool(true)
  ["option"]=>
  bool(true)
  ["abbreviation"]=>
  string(1) "N"
}
