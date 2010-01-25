--TEST--
Yaml YtsBasicTests - Sequence in a Mapping
--DESCRIPTION--
A value in a mapping can be a sequence.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('foo: whatever
bar:
 - uno
 - dos
'));
?>
--EXPECT--
array(2) {
  ["foo"]=>
  string(8) "whatever"
  ["bar"]=>
  array(2) {
    [0]=>
    string(3) "uno"
    [1]=>
    string(3) "dos"
  }
}
