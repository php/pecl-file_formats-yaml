--TEST--
yaml_emit -recursive structures
--SKIPIF--
<?php
if(!extension_loaded('yaml')) die('skip yaml n/a');
?>
--FILE--
<?php
$recursive = array(
  "a" => "a",
  "b" => "b",
  "c" => &$recursive,
);
var_dump(yaml_emit($recursive));

// clean up a little or we may see false memory leak reports
unset($recursive['c']);
?>
--EXPECT--
string(56) "---
a: a
b: b
c: &refid1
  a: a
  b: b
  c: *refid1
...
"
