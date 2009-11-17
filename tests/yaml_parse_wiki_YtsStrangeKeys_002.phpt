--TEST--
Yaml Wiki Test YtsStrangeKeys - Unquoted line noise key
--DESCRIPTION--
Check that unquoted line noise parses as key.
This requires the line noise be tame enough
to pass as an implicit plain key.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
-+!@#% : bar baz
'));
?>
--EXPECT--
array(1) {
  ["-+!@#%"]=>
  string(7) "bar baz"
}
