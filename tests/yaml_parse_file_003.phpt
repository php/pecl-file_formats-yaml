-TEST--
yaml_parse_file - ndocs
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$nd = 0;
yaml_parse_file(__DIR__ . '/yaml_parse_file_003.yaml', -1, $nd);
var_dump($nd);
--EXPECT--
int(2)
