--TEST--
yaml_parse parse errors
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('---
commercial-at: @text
'));
  var_dump(yaml_parse('---
grave-accent: `text
'));
?>
--EXPECTF--
Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 16), context while scanning for the next token (line 2, column 16) in %syaml_parse_003.php on line 4
bool(false)

Warning: yaml_parse(): scanning error encountered during parsing: found character that cannot start any token (line 2, column 15), context while scanning for the next token (line 2, column 15) in %syaml_parse_003.php on line 7
bool(false)
