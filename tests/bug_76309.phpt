--TEST--
Test PECL bug #76309
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
echo yaml_emit([
  'a' => '1.0',
  'b' => '2.',
  'c' => '3',
  'd' => '.',
  'e' => 1.0,
  'f' => 2.,
  'g' => 3,
] );
?>
--EXPECT--
---
a: "1.0"
b: "2."
c: "3"
d: .
e: 1.000000
f: 2.000000
g: 3
...
