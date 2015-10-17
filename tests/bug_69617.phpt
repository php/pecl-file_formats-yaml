--TEST--
Test PECL bug #69617
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
---
a: !php/object "O:1:\"A\":1:{s:3:\"one\";i:1;}"
...
YAML;

var_dump(yaml_parse($yaml_code));
?>
--EXPECT--
array(1) {
  ["a"]=>
  string(26) "O:1:"A":1:{s:3:"one";i:1;}"
}
