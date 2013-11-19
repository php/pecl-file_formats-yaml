--TEST--
Test PECL bug #61770
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(yaml_emit("\xc2"));
?>
--EXPECTF--
Warning: yaml_emit(): Invalid UTF-8 sequence in argument in %sbug_61770.php on line 2
bool(false)
