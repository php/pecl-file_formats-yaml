--TEST--
Yaml 1.2 Core Schema - float (no sexagesimal, no underscores)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
var_dump(yaml_parse('
# Valid 1.2 floats
canonical: 6.8523015e+5
fixed: 685230.15
negative: -1.5
leading_dot: .25
inf_lower: .inf
inf_upper: .INF
inf_neg: -.inf
nan_lower: .nan
nan_upper: .NaN
nan_all: .NAN
# NOT floats in 1.2 (they were in 1.1)
sexagesimal: 190:20:30.15
underscored: 685_230.15
'));
?>
--EXPECT--
array(12) {
  ["canonical"]=>
  float(685230.15)
  ["fixed"]=>
  float(685230.15)
  ["negative"]=>
  float(-1.5)
  ["leading_dot"]=>
  float(0.25)
  ["inf_lower"]=>
  float(INF)
  ["inf_upper"]=>
  float(INF)
  ["inf_neg"]=>
  float(-INF)
  ["nan_lower"]=>
  float(NAN)
  ["nan_upper"]=>
  float(NAN)
  ["nan_all"]=>
  float(NAN)
  ["sexagesimal"]=>
  string(12) "190:20:30.15"
  ["underscored"]=>
  string(10) "685_230.15"
}
