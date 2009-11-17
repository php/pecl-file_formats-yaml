--TEST--
Yaml Wiki Test YtsMapInSeq - Single key
--DESCRIPTION--
Test maps with just one key.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
- foo: bar
- baz: bug
'));
?>
--EXPECT--
array(2) {
  [0]=>
  array(1) {
    ["foo"]=>
    string(3) "bar"
  }
  [1]=>
  array(1) {
    ["baz"]=>
    string(3) "bug"
  }
}
