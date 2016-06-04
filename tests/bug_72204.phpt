--TEST--
Test PECL bug #72204
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$yaml_code = <<<YAML
foo: &ref
    bar: true
baz: &ref2
    <<: *ref
    bar: false
YAML;
$yaml = yaml_parse($yaml_code);
var_dump($yaml);
?>
--EXPECT--
array(2) {
  ["foo"]=>
  array(1) {
    ["bar"]=>
    bool(true)
  }
  ["baz"]=>
  array(1) {
    ["bar"]=>
    bool(false)
  }
}
