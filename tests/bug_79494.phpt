--TEST--
Test PECL bug #74949
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$data = array (
    'audio' =>
    array (
        'audioEnabled' =>
        array (
          0 => 132317787432502136,
          1 => 0,
    ),
    'eveampGain' =>
    array (
          0 => 132316833510704299,
          1 => 0.25,
        ),
    ),
);

print yaml_emit($data);
?>
--EXPECT--
---
audio:
  audioEnabled:
  - 132317787432502136
  - 0
  eveampGain:
  - 132316833510704299
  - 0.250000
...
