--TEST--
Yaml YtsBasicTests - Sequence-Mapping Shortcut
--DESCRIPTION--
If you are adding a mapping to a sequence, you
can place the mapping on the same line as the
dash as a shortcut.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('- work on YAML.py:
   - work on Store
'));
?>
--EXPECT--
array(1) {
  [0]=>
  array(1) {
    ["work on YAML.py"]=>
    array(1) {
      [0]=>
      string(13) "work on Store"
    }
  }
}
