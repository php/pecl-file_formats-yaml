--TEST--
Yaml 1.1 Spec - pairs
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
# Explicitly typed pairs.
Block tasks: !!pairs
  - meeting: with team.
  - meeting: with boss.
  - break: lunch.
  - meeting: with client.
Flow tasks: !!pairs [ meeting: with team, meeting: with boss ]
'));
?>
--EXPECT--
array(2) {
  ["Block tasks"]=>
  array(4) {
    [0]=>
    array(1) {
      ["meeting"]=>
      string(10) "with team."
    }
    [1]=>
    array(1) {
      ["meeting"]=>
      string(10) "with boss."
    }
    [2]=>
    array(1) {
      ["break"]=>
      string(6) "lunch."
    }
    [3]=>
    array(1) {
      ["meeting"]=>
      string(12) "with client."
    }
  }
  ["Flow tasks"]=>
  array(2) {
    [0]=>
    array(1) {
      ["meeting"]=>
      string(9) "with team"
    }
    [1]=>
    array(1) {
      ["meeting"]=>
      string(9) "with boss"
    }
  }
}
