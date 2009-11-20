--TEST--
yaml_parse - syck bug #11585
--CREDITS--
From syck bug #11585
http://pecl.php.net/bugs/bug.php?id=11585
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
define: &pointer_to_define
   - 1
   - 2
   - 3
reference: *broken_pointer_to_define
'));
?>
--EXPECTF--
Warning: yaml_parse(): alias broken_pointer_to_define is not registered in %syaml_parse_004.php on line 8
bool(false)
