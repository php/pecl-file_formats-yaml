--TEST--
Test PECL bug #69616
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
a: !php/object O:0:1
b: !php/object



YAML;

$yaml = yaml_parse($yaml_code);
?>
--EXPECTF--
Notice: yaml_parse(): Failed to unserialize class in %sbug_69616.php on line 10

Notice: yaml_parse(): Failed to unserialize class in %sbug_69616.php on line 10
