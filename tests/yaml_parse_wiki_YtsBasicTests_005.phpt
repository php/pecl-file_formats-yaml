--TEST--
Yaml Wiki Test YtsBasicTests - Simple Mapping
--DESCRIPTION--
You can add a keyed list "also known as a dictionary or
hash" to your document by placing each member of the
list on a new line, with a colon seperating the key
from its value.  In YAML, this type of list is called
a mapping.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('foo: whatever
bar: stuff
'));
?>
--EXPECT--
array(2) {
  ["foo"]=>
  string(8) "whatever"
  ["bar"]=>
  string(5) "stuff"
}
