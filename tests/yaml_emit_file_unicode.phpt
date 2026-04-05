--TEST--
yaml_emit_file - Unicode output
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$data = array(
    "city" => "香港",
    "greeting" => "Iñtërnâtiônàlizætiøn",
    "emoji" => "Hello 🌍",
);

$temp_filename = dirname(__FILE__) . '/yaml_emit_file_unicode.tmp';
var_dump(yaml_emit_file($temp_filename, $data));
$contents = file_get_contents($temp_filename);
echo $contents;

// verify roundtrip
$parsed = yaml_parse($contents);
var_dump($parsed["city"] === "香港");
var_dump($parsed["greeting"] === "Iñtërnâtiônàlizætiøn");
var_dump($parsed["emoji"] === "Hello 🌍");
?>
--CLEAN--
<?php
$temp_filename = dirname(__FILE__) . '/yaml_emit_file_unicode.tmp';
@unlink($temp_filename);
?>
--EXPECTF--
bool(true)
---
city: 香港
greeting: Iñtërnâtiônàlizætiøn
emoji: %s
...
bool(true)
bool(true)
bool(true)
