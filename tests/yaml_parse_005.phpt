--TEST--
yaml_parse syck bug #12656
--CREDITS--
From syck bug #12656
http://pecl.php.net/bugs/bug.php?id=12656
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
models : [ /usr/home/enovodk/data/core/easyworks/current/models/ ]
behaviors : [
/usr/home/enovodk/data/core/easyworks/current/models/behaviors/ ]
controllers : [
/usr/home/enovodk/data/core/easyworks/current/controllers/ ]
components : [
/usr/home/enovodk/data/core/easyworks/current/controllers/components/ ]
apis : [ /usr/home/enovodk/data/core/easyworks/current/controllers/api/
]
views : [ /usr/home/enovodk/data/core/easyworks/current/views/ ]
helpers : [ /usr/home/enovodk/data/core/easyworks/current/views/helpers/
]
libs : [ /usr/home/enovodk/data/core/easyworks/current/libs/ ]
plugins : [ /usr/home/enovodk/data/core/easyworks/current/plugins/ ]

'));
?>
--EXPECT--
array(9) {
  ["models"]=>
  array(1) {
    [0]=>
    string(53) "/usr/home/enovodk/data/core/easyworks/current/models/"
  }
  ["behaviors"]=>
  array(1) {
    [0]=>
    string(63) "/usr/home/enovodk/data/core/easyworks/current/models/behaviors/"
  }
  ["controllers"]=>
  array(1) {
    [0]=>
    string(58) "/usr/home/enovodk/data/core/easyworks/current/controllers/"
  }
  ["components"]=>
  array(1) {
    [0]=>
    string(69) "/usr/home/enovodk/data/core/easyworks/current/controllers/components/"
  }
  ["apis"]=>
  array(1) {
    [0]=>
    string(62) "/usr/home/enovodk/data/core/easyworks/current/controllers/api/"
  }
  ["views"]=>
  array(1) {
    [0]=>
    string(52) "/usr/home/enovodk/data/core/easyworks/current/views/"
  }
  ["helpers"]=>
  array(1) {
    [0]=>
    string(60) "/usr/home/enovodk/data/core/easyworks/current/views/helpers/"
  }
  ["libs"]=>
  array(1) {
    [0]=>
    string(51) "/usr/home/enovodk/data/core/easyworks/current/libs/"
  }
  ["plugins"]=>
  array(1) {
    [0]=>
    string(54) "/usr/home/enovodk/data/core/easyworks/current/plugins/"
  }
}
