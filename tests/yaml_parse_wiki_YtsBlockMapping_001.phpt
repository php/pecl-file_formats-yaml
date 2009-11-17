--TEST--
Yaml Wiki Test YtsBlockMapping - One Element Mapping
--DESCRIPTION--
A mapping with one key/value pair

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
foo: bar
'));
?>
--EXPECT--
array(1) {
  ["foo"]=>
  string(3) "bar"
}
