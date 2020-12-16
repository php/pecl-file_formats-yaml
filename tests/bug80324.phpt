--TEST--
Bug #80324 (Segfault in YAML with anonymous functions)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml = <<<YAML
- !env ENV
- !path PATH
YAML;

$result = yaml_parse($yaml, 0, $ndocs, array(
    '!env' => function ($str) {return $str;},
    '!path' => function ($str) {return $str;},
  ));

var_dump($result);
?>
--EXPECT--
array(2) {
  [0]=>
  string(3) "ENV"
  [1]=>
  string(4) "PATH"
}
