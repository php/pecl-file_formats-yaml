--TEST--
Test PECL bug #64019
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
; Ignore Deprecated: Using null as an array offset is deprecated...
error_reporting = E_ALL & ~E_DEPRECATED
--FILE--
<?php
$yaml_code = <<<YAML
configAnchors:
  - &wrongAnchor:   # This last colon (:) makes the script die
    configKey: configValue

config:
  <<: *wrongAnchor
YAML;

var_dump(yaml_parse($yaml_code));

$yaml_code = <<<YAML
config:
  <<:
YAML;

var_dump(yaml_parse($yaml_code));
?>
--EXPECT--
array(2) {
  ["configAnchors"]=>
  array(1) {
    [0]=>
    array(2) {
      [""]=>
      NULL
      ["configKey"]=>
      string(11) "configValue"
    }
  }
  ["config"]=>
  array(1) {
    ["<<"]=>
    NULL
  }
}
array(1) {
  ["config"]=>
  array(1) {
    ["<<"]=>
    NULL
  }
}
