--TEST--
Yaml Wiki Test YtsMapInSeq - Multiple keys
--DESCRIPTION--
Test a map with multiple keys.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
- foo: bar
  baz: bug
'));
?>
--EXPECT--
array(1) {
  [0]=>
  array(2) {
    ["foo"]=>
    string(3) "bar"
    ["baz"]=>
    string(3) "bug"
  }
}
