--TEST--
Test PECL strange alias bug
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php

$yaml_code = <<<YAML
basic:
  - &external {value: 22 }
keylevel0:
  keylevel1:
    value: 1
  keylevel1.1:
    keylevel2: &strange
      value: 2
      <<: *external
YAML;
var_dump(yaml_parse($yaml_code));
?>
--EXPECT--
array(2) {
  ["basic"]=>
  array(1) {
    [0]=>
    array(1) {
      ["value"]=>
      int(22)
    }
  }
  ["keylevel0"]=>
  array(2) {
    ["keylevel1"]=>
    array(1) {
      ["value"]=>
      int(1)
    }
    ["keylevel1.1"]=>
    array(1) {
      ["keylevel2"]=>
      array(1) {
        ["value"]=>
        int(2)
      }
    }
  }
}

