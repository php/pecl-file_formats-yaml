--TEST--
Yaml 1.1 Spec - map
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
# Unordered set of key: value pairs.
Block style: !!map
  Clark : Evans
  Brian : Ingerson
  Oren  : Ben-Kiki
Flow style: !!map { Clark: Evans, Brian: Ingerson, Oren: Ben-Kiki }
'));
?>
--EXPECT--
array(2) {
  ["Block style"]=>
  array(3) {
    ["Clark"]=>
    string(5) "Evans"
    ["Brian"]=>
    string(8) "Ingerson"
    ["Oren"]=>
    string(8) "Ben-Kiki"
  }
  ["Flow style"]=>
  array(3) {
    ["Clark"]=>
    string(5) "Evans"
    ["Brian"]=>
    string(8) "Ingerson"
    ["Oren"]=>
    string(8) "Ben-Kiki"
  }
}
