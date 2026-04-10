--TEST--
Yaml 1.2 Core Schema - bool (only true/false recognized)
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_schema=2
--FILE--
<?php
var_dump(yaml_parse('
# These are booleans in 1.2
a: true
b: True
c: TRUE
d: false
e: False
f: FALSE
# These are NOT booleans in 1.2 (they were in 1.1)
g: yes
h: no
i: on
j: off
k: y
l: n
m: Yes
n: No
'));
?>
--EXPECT--
array(14) {
  ["a"]=>
  bool(true)
  ["b"]=>
  bool(true)
  ["c"]=>
  bool(true)
  ["d"]=>
  bool(false)
  ["e"]=>
  bool(false)
  ["f"]=>
  bool(false)
  ["g"]=>
  string(3) "yes"
  ["h"]=>
  string(2) "no"
  ["i"]=>
  string(2) "on"
  ["j"]=>
  string(3) "off"
  ["k"]=>
  string(1) "y"
  ["l"]=>
  string(1) "n"
  ["m"]=>
  string(3) "Yes"
  ["n"]=>
  string(2) "No"
}
