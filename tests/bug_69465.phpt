--TEST--
Test PECL bug #69465
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=UTC
--FILE--
<?php
$yaml_code = <<<YAML
date1: 2015-05-15
date2: "2015-05-15"
date3: ! 2015-05-15
bool1: true
bool2: "true"
bool3: ! true
int1: 1
int2: "1"
int3: ! 1
float1: 1.5
float2: "1.5"
float3: ! 1.5
YAML;

var_dump(yaml_parse($yaml_code));
?>
--EXPECT--
array(12) {
  ["date1"]=>
  int(1431648000)
  ["date2"]=>
  string(10) "2015-05-15"
  ["date3"]=>
  string(10) "2015-05-15"
  ["bool1"]=>
  bool(true)
  ["bool2"]=>
  string(4) "true"
  ["bool3"]=>
  string(4) "true"
  ["int1"]=>
  int(1)
  ["int2"]=>
  string(1) "1"
  ["int3"]=>
  string(1) "1"
  ["float1"]=>
  float(1.5)
  ["float2"]=>
  string(3) "1.5"
  ["float3"]=>
  string(3) "1.5"
}
