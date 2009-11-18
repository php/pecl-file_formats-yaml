--TEST--
yaml_parse multiple documents
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
---
time: !!str 20:03:20
player: Sammy Sosa
action: strike (miss)
...

%YAML 1.1
---
time: "20:03:47"
player: Sammy Sosa
action: grand slam
...
', -1));
?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["time"]=>
    string(8) "20:03:20"
    ["player"]=>
    string(10) "Sammy Sosa"
    ["action"]=>
    string(13) "strike (miss)"
  }
  [1]=>
  array(3) {
    ["time"]=>
    string(8) "20:03:47"
    ["player"]=>
    string(10) "Sammy Sosa"
    ["action"]=>
    string(10) "grand slam"
  }
}
