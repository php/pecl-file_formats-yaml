--TEST--
Yaml 1.1 Spec - set
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('baseball players: !!set
  ? Mark McGwire
  ? Sammy Sosa
  ? Ken Griffey
'));
?>
--EXPECT--
array(1) {
  ["baseball players"]=>
  array(3) {
    ["Mark McGwire"]=>
    NULL
    ["Sammy Sosa"]=>
    NULL
    ["Ken Griffey"]=>
    NULL
  }
}
