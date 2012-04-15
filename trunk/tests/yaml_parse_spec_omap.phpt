--TEST--
Yaml 1.1 Spec - omap
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
# Explicitly typed ordered map (dictionary).
Bestiary: !!omap
  - aardvark: African pig-like ant eater. Ugly.
  - anteater: South-American ant eater. Two species.
  - anaconda: South-American constrictor snake. Scaly.
  # Etc.
# Flow style
Numbers: !!omap [ one: 1, two: 2, three : 3 ]
'));
?>
--EXPECT--
array(2) {
  ["Bestiary"]=>
  array(3) {
    [0]=>
    array(1) {
      ["aardvark"]=>
      string(33) "African pig-like ant eater. Ugly."
    }
    [1]=>
    array(1) {
      ["anteater"]=>
      string(38) "South-American ant eater. Two species."
    }
    [2]=>
    array(1) {
      ["anaconda"]=>
      string(40) "South-American constrictor snake. Scaly."
    }
  }
  ["Numbers"]=>
  array(3) {
    [0]=>
    array(1) {
      ["one"]=>
      int(1)
    }
    [1]=>
    array(1) {
      ["two"]=>
      int(2)
    }
    [2]=>
    array(1) {
      ["three"]=>
      int(3)
    }
  }
}
