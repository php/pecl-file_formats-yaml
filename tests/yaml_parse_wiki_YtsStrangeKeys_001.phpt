--TEST--
Yaml Wiki Test YtsStrangeKeys - Quoted line noise key
--DESCRIPTION--
Check that quoted line noise parses as key.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
"!@#%" : bar baz
'));
?>
--EXPECT--
array(1) {
  ["!@#%"]=>
  string(7) "bar baz"
}
