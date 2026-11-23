--TEST--
yaml_emit - object that cannot be serialized
--SKIPIF--
<?php
if (!extension_loaded('yaml')) die('skip yaml n/a');
if (PHP_VERSION_ID < 70400) die('skip requires PHP 7.4+');
?>
--FILE--
<?php
class NotSerializable {
  public function __serialize(): array {
    throw new Exception('not serializable');
  }
}

foreach ([fn() => 1, new NotSerializable()] as $value) {
  try {
    var_dump(yaml_emit([$value]));
  } catch (Throwable $e) {
    echo get_class($e), ': ', $e->getMessage(), "\n";
  }
}

// a serializable object is still emitted
echo yaml_emit([new ArrayObject([1])]);
?>
--EXPECT--
Exception: Serialization of 'Closure' is not allowed
Exception: not serializable
---
- !php/object "O:11:\"ArrayObject\":4:{i:0;i:0;i:1;a:1:{i:0;i:1;}i:2;a:0:{}i:3;N;}"
...
