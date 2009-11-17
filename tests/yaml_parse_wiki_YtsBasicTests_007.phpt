--TEST--
Yaml Wiki Test YtsBasicTests - Nested Mappings
--DESCRIPTION--
A value in a mapping can be another mapping.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('foo: whatever
bar:
 fruit: apple
 name: steve
 sport: baseball
'));
?>
--EXPECT--
array(2) {
  ["foo"]=>
  string(8) "whatever"
  ["bar"]=>
  array(3) {
    ["fruit"]=>
    string(5) "apple"
    ["name"]=>
    string(5) "steve"
    ["sport"]=>
    string(8) "baseball"
  }
}
