--TEST--
Memory leak check
--CREDITS--
Gleb Svitelskiy <gleb.svitelskiy@gmail.com>
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
function tag_callback($value, $tag, $flags) {
    return 'callback value';
}

$base_memory = memory_get_usage();

$yamlString ="---\nkey: !value\n...\n";
$ndocs = null;
yaml_parse($yamlString, 0, $ndocs, ['!value' => 'tag_callback']);
unset($yamlString, $ndocs);

$ndocs = null;
yaml_parse_file(__DIR__.'/bug_74799.yaml', 0, $ndocs, ['!value' => 'tag_callback']);
unset($ndocs);

$array = ['key' => 'value'];
yaml_emit($array);
unset($array);

$array = ['key' => 'value'];
yaml_emit_file(__DIR__.'/bug_74799.tmp', $array);
unset($array);

var_dump($base_memory == memory_get_usage());
?>
--CLEAN--
<?php
unlink(__DIR__.'/bug_74799.tmp');
?>
--EXPECT--
bool(true)
