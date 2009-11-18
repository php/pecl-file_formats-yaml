--TEST--
Yaml 1.1 Spec - timestamp
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=GMT
--FILE--
<?php
  var_dump(yaml_parse('
canonical:        2001-12-15T02:59:43.1Z
valid iso8601:    2001-12-14t21:59:43.10-05:00
space separated:  2001-12-14 21:59:43.10 -5
no time zone (Z): 2001-12-15 2:59:43.10
date (00:00:00Z): 2002-12-14
'));
?>
--EXPECT--
array(5) {
  ["canonical"]=>
  int(1008385183)
  ["valid iso8601"]=>
  int(1008385183)
  ["space separated"]=>
  int(1008385183)
  ["no time zone (Z)"]=>
  int(1008385183)
  ["date (00:00:00Z)"]=>
  int(1039824000)
}
