--TEST--
yaml_parse - serializable
--SKIPIF--
<?php
if(!extension_loaded('yaml')) die('skip yaml n/a');
?>
--FILE--
<?php
class A {
  public $one = 1;

  public function show_one() {
    echo $this->one;
  }
}

var_dump(yaml_parse('
---
a: !php/object "O:1:\"A\":1:{s:3:\"one\";i:1;}"
...
'));
?>
--EXPECTF--
array(1) {
  ["a"]=>
  object(A)#%d (1) {
    ["one"]=>
    int(1)
  }
}

