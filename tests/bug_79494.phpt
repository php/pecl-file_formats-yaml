--TEST--
Test PECL bug #74949
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
serialize_precision=-1
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
--EXPECTF--
---
audio:
  audioEnabled:
  - %r(132317787432502136|1\.3231778743250214E\+17)%r
  - 0
  eveampGain:
  - %r(132316833510704299|1\.323168335107043E\+17)%r
  - 0.25
...
