--TEST--
yaml_emit - serializable
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
$a = new A;

var_dump(yaml_emit(array('a' => $a)));
?>
--EXPECT--
string(56) "---
a: !php/object "O:1:\"A\":1:{s:3:\"one\";i:1;}"
...
"
