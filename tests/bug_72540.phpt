--TEST--
Test PECL bug #72540
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$tmpfname = tempnam(sys_get_temp_dir(), 'bug_72540.yaml');
var_dump(yaml_parse_url($tmpfname));
unlink($tmpfname);
?>
--EXPECT--
bool(false)
