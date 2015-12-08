--TEST--
Test tag callback of aliased list
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
- &scalar !transform false
- *scalar
- &sequence !transform [false]
- *sequence
- &mapping !transform {k: false}
- *mapping
- &unused [true]
YAML;

function transform()
{
    return true;
}

var_dump(yaml_parse($yaml_code, 0, $count, array(
    '!transform' => 'transform',
)));
?>
--EXPECTF--
array(7) {
  [0]=>
  &bool(true)
  [1]=>
  &bool(true)
  [2]=>
  &bool(true)
  [3]=>
  &bool(true)
  [4]=>
  &bool(true)
  [5]=>
  &bool(true)
  [6]=>
  array(1) {
    [0]=>
    bool(true)
  }
}
