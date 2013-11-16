--TEST--
Test PECL bug #64694
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
[a]: 1
YAML;

var_dump(yaml_parse($yaml_code));

$yaml_code = <<<YAML
"[a]": 1
YAML;

var_dump(yaml_parse($yaml_code));
?>
--EXPECTF--
Warning: yaml_parse(): Failed to convert array to string in %sbug_64694.php on line 6
bool(false)
array(1) {
  ["[a]"]=>
  int(1)
}
