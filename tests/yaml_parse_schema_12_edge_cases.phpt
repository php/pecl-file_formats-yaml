--TEST--
Yaml 1.2 Core Schema - edge cases (0e5, trailing dot, explicit bool tag)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
// Fix 1: 0e5 / 0E+10 should be valid floats
// Fix 2: trailing dot (1., 0.) should be valid floats
// Fix 3: explicit !!bool tag should parse true/false correctly
var_dump(yaml_parse('
# 0-prefixed exponent floats
zero_exp: 0e5
zero_exp_plus: 0E+10
zero_exp_neg: 0e-3
# trailing dot floats
one_dot: 1.
zero_dot: 0.
neg_dot: -1.
# trailing dot with exponent
dot_exp: 1.e2
# bare dot is NOT a float
# explicit bool tags
bool_false: !!bool "false"
bool_true: !!bool "true"
bool_FALSE: !!bool "FALSE"
bool_TRUE: !!bool "TRUE"
bool_zero: !!bool "0"
bool_one: !!bool "1"
'));
?>
--EXPECT--
array(13) {
  ["zero_exp"]=>
  float(0)
  ["zero_exp_plus"]=>
  float(0)
  ["zero_exp_neg"]=>
  float(0)
  ["one_dot"]=>
  float(1)
  ["zero_dot"]=>
  float(0)
  ["neg_dot"]=>
  float(-1)
  ["dot_exp"]=>
  float(100)
  ["bool_false"]=>
  bool(false)
  ["bool_true"]=>
  bool(true)
  ["bool_FALSE"]=>
  bool(false)
  ["bool_TRUE"]=>
  bool(true)
  ["bool_zero"]=>
  bool(false)
  ["bool_one"]=>
  bool(true)
}
