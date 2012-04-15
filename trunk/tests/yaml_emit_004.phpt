--TEST--
yaml_emit - datetime
--SKIPIF--
<?php
if(!extension_loaded('yaml')) die('skip yaml n/a');
if(!class_exists('DateTime')) die('skip DateTime n/a');
?>
--INI--
date.timezone=GMT
--FILE--
<?php
$d = new DateTime('2008-08-03T14:52:10Z');
var_dump(yaml_emit($d));
?>
--EXPECT--
string(33) "--- 2008-08-03T14:52:10+0000
...
"
