--TEST--
Yaml YtsBasicTests - Nested Sequences
--DESCRIPTION--
You can include a sequence within another
sequence by giving the sequence an empty
dash, followed by an indented list.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('-
 - foo
 - bar
 - baz
'));
?>
--EXPECT--
array(1) {
  [0]=>
  array(3) {
    [0]=>
    string(3) "foo"
    [1]=>
    string(3) "bar"
    [2]=>
    string(3) "baz"
  }
}
