--TEST--
yaml - emit -> parse roundtrip with datetime
--SKIPIF--
<?php
if(!extension_loaded('yaml')) die('skip yaml n/a');
if(!class_exists('DateTime')) die('skip DateTime n/a');
?>
--INI--
date.timezone=GMT
yaml.decode_timestamp=2
--FILE--
<?php
$original = array(
  "canonical" => new DateTime("2001-12-15T02:59:43.1Z"),
  "valid iso8601" => new DateTime("2001-12-14t21:59:43.10-05:00"),
  "alternate iso8601" => new DateTime("2001-12-14T21:59:43.10-0500"),
  "space separated" => new DateTime("2001-12-14 21:59:43.10 -5"),
  "no time zone (Z)" => new DateTime("2001-12-15 2:59:43.10"),
  "date (0000:00Z)" => new DateTime("2002-12-14"),
);

$yaml = yaml_emit($original);
var_dump($yaml);
$parsed = yaml_parse($yaml);
var_dump($parsed == $original);
?>
--EXPECT--
string(255) "---
canonical: 2001-12-15T02:59:43+0000
valid iso8601: 2001-12-14T21:59:43-0500
alternate iso8601: 2001-12-14T21:59:43-0500
space separated: 2001-12-14T21:59:43-0500
no time zone (Z): 2001-12-15T02:59:43+0000
date (0000:00Z): 2002-12-14T00:00:00+0000
...
"
bool(true)
