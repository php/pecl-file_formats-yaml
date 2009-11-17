--TEST--
Yaml 1.1 Spec - float
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
canonical: 6.8523015e+5
exponentioal: 685.230_15e+03
fixed: 685_230.15
sexagesimal: 190:20:30.15
negative infinity: -.inf
not a number: .NaN
'));
?>
--EXPECT--
array(6) {
  ["canonical"]=>
  float(685230.15)
  ["exponentioal"]=>
  float(685230.15)
  ["fixed"]=>
  float(685230.15)
  ["sexagesimal"]=>
  float(685230.15)
  ["negative infinity"]=>
  float(-INF)
  ["not a number"]=>
  float(NAN)
}
