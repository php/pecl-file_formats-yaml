--TEST--
Yaml 1.1 Spec - null
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
# A document may be null.
---
---
# This mapping has four keys,
# one has a value.
empty:
canonical: ~
english: null
~: null key
---
# This sequence has five
# entries, two have values.
sparse:
  - ~
  - 2nd entry
  -
  - 4th entry
  - Null
', -1));
?>
--EXPECT--
array(3) {
  [0]=>
  NULL
  [1]=>
  array(4) {
    ["empty"]=>
    NULL
    ["canonical"]=>
    NULL
    ["english"]=>
    NULL
    [""]=>
    string(8) "null key"
  }
  [2]=>
  array(1) {
    ["sparse"]=>
    array(5) {
      [0]=>
      NULL
      [1]=>
      string(9) "2nd entry"
      [2]=>
      NULL
      [3]=>
      string(9) "4th entry"
      [4]=>
      NULL
    }
  }
}
