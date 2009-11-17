--TEST--
Yaml Wiki Test YtsBlockMapping - Colons aligned
--DESCRIPTION--
Spaces can come before the ': ' key/value separator.

--CREDITS--
Bryan Davis bpd@keynetics.com
# yaml.kwiki.org import/conversion
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
red   : baron
white : walls
blue  : berries
'));
?>
--EXPECT--
array(3) {
  ["red"]=>
  string(5) "baron"
  ["white"]=>
  string(5) "walls"
  ["blue"]=>
  string(7) "berries"
}
