--TEST--
Test PECL bug #260628
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$data = <<<YAML
#yaml
---
data: !mytag
  - look upper
...
YAML;

function tag_callback ($value, $tag, $flags) {
  echo "-- callback value --\n";
  var_dump($value);
  var_dump($tag);
  var_dump($flags);
  echo "-- end callback value --\n";
  return array(
      'data' => $value,
      'another' => 'test',
    );
}

/* baseline. do like operation in native php. */
$native = array(
  "data" => array("look upper"),
);
$native["data"] = tag_callback($native["data"], "!mytag", 0);

echo "-- native value --\n";
var_dump($native);
echo "-- end native value --\n";

$cnt = null;
$array = yaml_parse($data, 0, $cnt, array(
    '!mytag' => 'tag_callback',
  ));

echo "-- parsed value --\n";
var_dump($array);
echo "-- end parsed value --\n";
?>
--EXPECT--
-- callback value --
array(1) {
  [0]=>
  string(10) "look upper"
}
string(6) "!mytag"
int(0)
-- end callback value --
-- native value --
array(1) {
  ["data"]=>
  array(2) {
    ["data"]=>
    array(1) {
      [0]=>
      string(10) "look upper"
    }
    ["another"]=>
    string(4) "test"
  }
}
-- end native value --
-- callback value --
array(1) {
  [0]=>
  string(10) "look upper"
}
string(6) "!mytag"
int(0)
-- end callback value --
-- parsed value --
array(1) {
  ["data"]=>
  array(2) {
    ["data"]=>
    array(1) {
      [0]=>
      string(10) "look upper"
    }
    ["another"]=>
    string(4) "test"
  }
}
-- end parsed value --
