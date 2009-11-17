--TEST--
Yaml Wiki Test YtsBasicTests - Mixed Sequences
--DESCRIPTION--
Sequences can contain any YAML data,
including strings and other sequences.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('- apple
-
 - foo
 - bar
 - x123
- banana
- carrot
'));
?>
--EXPECT--
array(4) {
  [0]=>
  string(5) "apple"
  [1]=>
  array(3) {
    [0]=>
    string(3) "foo"
    [1]=>
    string(3) "bar"
    [2]=>
    string(4) "x123"
  }
  [2]=>
  string(6) "banana"
  [3]=>
  string(6) "carrot"
}
