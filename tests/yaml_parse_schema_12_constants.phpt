--TEST--
Yaml 1.2 Schema - PHP constants defined
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(YAML_SCHEMA_DEFAULT);
var_dump(YAML_SCHEMA_YAML_1_1);
var_dump(YAML_SCHEMA_CORE_1_2);
?>
--EXPECT--
int(0)
int(1)
int(2)
