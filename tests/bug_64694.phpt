--TEST--
Test PECL bug #64694
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
"[a]": 1
YAML;

var_dump(yaml_parse($yaml_code));

$yaml_code = <<<YAML
[a]: 1
YAML;

var_dump(yaml_parse($yaml_code));
?>
--EXPECT--
array(1) {
  ["[a]"]=>
  int(1)
}
array(1) {
  ["a:1:{i:0;s:1:"a";}"]=>
  int(1)
}
