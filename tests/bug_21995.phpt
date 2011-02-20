--TEST--
Test PECL bug #21995
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
function yaml_cbk ($a) {
  var_dump($a);
  return $a;
}

$yaml_code = <<<YAML
boo: doo
a: [1,2,3,4]
d: []
YAML;

$yaml = yaml_parse($yaml_code, 0, &$ndocs, array(
    YAML_STR_TAG => "yaml_cbk",
    ));
?>
--EXPECT--
string(3) "boo"
string(3) "doo"
string(1) "a"
string(1) "d"
