--TEST--
Yaml 1.1 Spec - merge
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(yaml_parse('
---
- &CENTER { x: 1, "y": 2 }
- &LEFT { x: 0, "y": 2 }
- &BIG { r: 10 }
- &SMALL { r: 1 }

# All the following maps are equal:

- # Explicit keys
  x: 1
  "y": 2
  r: 10
  label: center/big

- # Merge one map
  << : *CENTER
  r: 10
  label: center/big

- # Merge multiple maps
  << : [ *CENTER, *BIG ]
  label: center/big

- # Override
  << : [ *BIG, *LEFT, *SMALL ]
  x: 1
  label: center/big
'));
?>
--EXPECT--
array(8) {
  [0]=>
  array(2) {
    ["x"]=>
    int(1)
    ["y"]=>
    int(2)
  }
  [1]=>
  array(2) {
    ["x"]=>
    int(0)
    ["y"]=>
    int(2)
  }
  [2]=>
  array(1) {
    ["r"]=>
    int(10)
  }
  [3]=>
  array(1) {
    ["r"]=>
    int(1)
  }
  [4]=>
  array(4) {
    ["x"]=>
    int(1)
    ["y"]=>
    int(2)
    ["r"]=>
    int(10)
    ["label"]=>
    string(10) "center/big"
  }
  [5]=>
  array(4) {
    ["x"]=>
    int(1)
    ["y"]=>
    int(2)
    ["r"]=>
    int(10)
    ["label"]=>
    string(10) "center/big"
  }
  [6]=>
  array(4) {
    ["x"]=>
    int(1)
    ["y"]=>
    int(2)
    ["r"]=>
    int(10)
    ["label"]=>
    string(10) "center/big"
  }
  [7]=>
  array(4) {
    ["r"]=>
    int(10)
    ["x"]=>
    int(1)
    ["y"]=>
    int(2)
    ["label"]=>
    string(10) "center/big"
  }
}
