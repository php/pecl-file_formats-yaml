--TEST--
Yaml Wiki Test YtsMapInSeq - Strange keys
--DESCRIPTION--
Test a map with "line noise" keys.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
- "!@#$" : foo
  -+!@ :   bar
'));
?>
--EXPECT--
array(1) {
  [0]=>
  array(2) {
    ["!@#$"]=>
    string(3) "foo"
    ["-+!@"]=>
    string(3) "bar"
  }
}
