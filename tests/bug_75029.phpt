--TEST--
Test PECL bug #75029
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(yaml_parse(''));
var_dump(yaml_parse('# Empty', -1));
$doc = <<<EOD
doc
---
---
doc
EOD;
var_dump(yaml_parse($doc, -1));
var_dump(yaml_parse($doc, 1));
?>
--EXPECT--
NULL
array(1) {
  [0]=>
  NULL
}
array(3) {
  [0]=>
  string(3) "doc"
  [1]=>
  NULL
  [2]=>
  string(3) "doc"
}
NULL
