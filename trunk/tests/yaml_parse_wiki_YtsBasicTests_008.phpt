--TEST--
Yaml YtsBasicTests - Mixed Mapping
--DESCRIPTION--
A mapping can contain any assortment
of mappings and sequences as values.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('foo: whatever
bar:
 -
  fruit: apple
  name: steve
  sport: baseball
 - more
 -
  python: rocks
  perl: papers
  ruby: scissorses
'));
?>
--EXPECT--
array(2) {
  ["foo"]=>
  string(8) "whatever"
  ["bar"]=>
  array(3) {
    [0]=>
    array(3) {
      ["fruit"]=>
      string(5) "apple"
      ["name"]=>
      string(5) "steve"
      ["sport"]=>
      string(8) "baseball"
    }
    [1]=>
    string(4) "more"
    [2]=>
    array(3) {
      ["python"]=>
      string(5) "rocks"
      ["perl"]=>
      string(6) "papers"
      ["ruby"]=>
      string(10) "scissorses"
    }
  }
}
