--TEST--
Test PECL bug #74886
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(yaml_parse('
- &REF { "x": 1 }
- # scalar
  << : 0x7FFFFFFF
- # scalar in sequence len 1
  << : [ 0x7FFFFFFF ]
- # scalar in sequence len 2
  << : [ *REF, 0x7FFFFFFF ]

- &SCALAR_REF some-string
- # scalar ref
  << : *SCALAR_REF
- # scalar ref in sequence len 1
  << : [ *SCALAR_REF ]
- # scalar ref in sequence len 2
  << : [ *REF, *SCALAR_REF ]
'));
?>
--EXPECTF--
Warning: yaml_parse(): expected a mapping for merging, but found scalar (line 6, column 22) in %sbug_74886.php on line 2

Warning: yaml_parse(): expected a mapping for merging, but found scalar (line 8, column 28) in %sbug_74886.php on line 2

Warning: yaml_parse(): expected a mapping for merging, but found scalar (line 14, column 23) in %sbug_74886.php on line 2

Warning: yaml_parse(): expected a mapping for merging, but found scalar (line 16, column 29) in %sbug_74886.php on line 2
array(8) {
  [0]=>
  array(1) {
    ["x"]=>
    int(1)
  }
  [1]=>
  array(1) {
    ["<<"]=>
    int(2147483647)
  }
  [2]=>
  array(0) {
  }
  [3]=>
  array(1) {
    ["x"]=>
    int(1)
  }
  [4]=>
  &string(11) "some-string"
  [5]=>
  array(1) {
    ["<<"]=>
    &string(11) "some-string"
  }
  [6]=>
  array(0) {
  }
  [7]=>
  array(1) {
    ["x"]=>
    int(1)
  }
}
