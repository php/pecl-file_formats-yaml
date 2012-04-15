--TEST--
Test PECL bug #59860
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
/**
 * Parsing callback for yaml tag.
 * @param mixed $value Data from yaml file
 * @param string $tag Tag that triggered callback
 * @param int $flags Scalar entity style (see YAML_*_SCALAR_STYLE)
 * @return mixed Value that YAML parser should emit for the given value
 */
function tag_callback ($value, $tag, $flags) {
  var_dump(func_get_args());
  return $tag;
}

$yaml = <<<YAML
implicit_map:
  a: b
explicit_map: !!map
  c: d
implicit_seq: [e, f]
explicit_seq: !!seq [g, h]
YAML;

yaml_parse($yaml, 0, $ndocs, array(
    YAML_MAP_TAG => 'tag_callback',
    YAML_SEQ_TAG => 'tag_callback',
  ));
?>
--EXPECT--
array(3) {
  [0]=>
  array(1) {
    ["a"]=>
    string(1) "b"
  }
  [1]=>
  string(21) "tag:yaml.org,2002:map"
  [2]=>
  int(0)
}
array(3) {
  [0]=>
  array(1) {
    ["c"]=>
    string(1) "d"
  }
  [1]=>
  string(21) "tag:yaml.org,2002:map"
  [2]=>
  int(0)
}
array(3) {
  [0]=>
  array(2) {
    [0]=>
    string(1) "e"
    [1]=>
    string(1) "f"
  }
  [1]=>
  string(21) "tag:yaml.org,2002:seq"
  [2]=>
  int(0)
}
array(3) {
  [0]=>
  array(2) {
    [0]=>
    string(1) "g"
    [1]=>
    string(1) "h"
  }
  [1]=>
  string(21) "tag:yaml.org,2002:seq"
  [2]=>
  int(0)
}
array(3) {
  [0]=>
  array(4) {
    ["implicit_map"]=>
    string(21) "tag:yaml.org,2002:map"
    ["explicit_map"]=>
    string(21) "tag:yaml.org,2002:map"
    ["implicit_seq"]=>
    string(21) "tag:yaml.org,2002:seq"
    ["explicit_seq"]=>
    string(21) "tag:yaml.org,2002:seq"
  }
  [1]=>
  string(21) "tag:yaml.org,2002:map"
  [2]=>
  int(0)
}
