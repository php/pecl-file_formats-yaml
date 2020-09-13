--TEST--
yaml_parse_file - error cases
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=GMT
--FILE--
<?php
try {
  // PHP7 emits a Warning here
  yaml_parse_file(NULL);
} catch (ValueError $e) {
  // PHP8 raises this exception
  echo "\nWarning: yaml_parse_file(): {$e->getMessage()} in " . __FILE__ . " on line 4\n";
}
try {
  // PHP7 emits a Warning here
  yaml_parse_file('');
} catch (ValueError $e) {
  // PHP8 raises this exception
  echo "\nWarning: yaml_parse_file(): {$e->getMessage()} in " . __FILE__ . " on line 11\n";
}
yaml_parse_file('invalid');
try {
  // PHP7 emits a Warning here
  yaml_parse_file();
} catch (ArgumentCountError $e) {
  // PHP8 raises this exception
  echo "\nWarning: {$e->getMessage()} in " . __FILE__ . " on line 19\n";
}
--EXPECTF--
Warning: yaml_parse_file(): %r(Filename|Path)%r cannot be empty in %s on line %d

Warning: yaml_parse_file(): %r(Filename|Path)%r cannot be empty in %s on line %d

Warning: yaml_parse_file(invalid): %r[Ff]%railed to open stream: No such file or directory in %s on line %d

Warning: yaml_parse_file() expects at least 1 %r(argument|parameter)%r, 0 given in %s on line %d
