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
--EXPECTF--
array(1) {
  ["[a]"]=>
  int(1)
}

Warning: yaml_parse(): Illegal offset type array (line 1, column 7) in %s/bug_64694.php on line 12
array(0) {
}
