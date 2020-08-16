--TEST--
Test PECL bug #61923
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
serialize_precision=-1
--FILE--
<?php
$yaml_code = <<<YAML
---
strings:
  - '1:0'
  - '0:1'
  - '1:0:0'
  - '+1:0:0'
  - '-1:0:0'
  - ':01'
  - ':1'
  - '18:53:17.00037'
numbers:
  - 1:0
  - 0:1
  - 1:0:0
  - +1:0:0
  - -1:0:0
  - :01
  - :1
  - 18:53:17.00037
YAML;

$parsed = yaml_parse($yaml_code);
var_dump($parsed);
var_dump(yaml_emit($parsed));
?>
--EXPECT--
array(2) {
  ["strings"]=>
  array(8) {
    [0]=>
    string(3) "1:0"
    [1]=>
    string(3) "0:1"
    [2]=>
    string(5) "1:0:0"
    [3]=>
    string(6) "+1:0:0"
    [4]=>
    string(6) "-1:0:0"
    [5]=>
    string(3) ":01"
    [6]=>
    string(2) ":1"
    [7]=>
    string(14) "18:53:17.00037"
  }
  ["numbers"]=>
  array(8) {
    [0]=>
    int(60)
    [1]=>
    int(1)
    [2]=>
    int(3600)
    [3]=>
    int(3600)
    [4]=>
    int(-3600)
    [5]=>
    int(1)
    [6]=>
    int(1)
    [7]=>
    float(67997.00037)
  }
}
string(161) "---
strings:
- "1:0"
- "0:1"
- "1:0:0"
- "+1:0:0"
- "-1:0:0"
- ":01"
- ":1"
- "18:53:17.00037"
numbers:
- 60
- 1
- 3600
- 3600
- -3600
- 1
- 1
- 67997.00037
...
"
