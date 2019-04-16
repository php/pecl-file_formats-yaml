--TEST--
yaml_parse_file - error cases
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=GMT
--FILE--
<?php
yaml_parse_file(NULL);
yaml_parse_file('');
yaml_parse_file('invalid');
try {
  // PHP7 emits a Warning here
  yaml_parse_file();
} catch (ArgumentCountError $e) {
  // PHP8 raises this exception
  echo "\nWarning: {$e->getMessage()} in " . __FILE__ . " on line 7\n";
}
--EXPECTF--
Warning: yaml_parse_file(): Filename cannot be empty in %s on line %d

Warning: yaml_parse_file(): Filename cannot be empty in %s on line %d

Warning: yaml_parse_file(invalid): failed to open stream: No such file or directory in %s on line %d

Warning: yaml_parse_file() expects at least 1 parameter, 0 given in %s on line %d
