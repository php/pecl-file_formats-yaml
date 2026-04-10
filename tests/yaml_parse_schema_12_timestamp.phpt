--TEST--
Yaml 1.2 Core Schema - no implicit timestamp detection
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
yaml.decode_timestamp=1
--FILE--
<?php
var_dump(yaml_parse('
# These are strings in 1.2 Core Schema (no implicit timestamp)
date: 2024-01-15
datetime: 2024-01-15T10:30:00Z
'));
?>
--EXPECT--
array(2) {
  ["date"]=>
  string(10) "2024-01-15"
  ["datetime"]=>
  string(20) "2024-01-15T10:30:00Z"
}
