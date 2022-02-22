--TEST--
yaml_parse_url - general
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=GMT
--FILE--
<?php
var_dump(yaml_parse_url('file://' . __DIR__ . '/yaml_parse_file_001.yaml'));
?>
--EXPECT--
array(8) {
  ["invoice"]=>
  int(34843)
  ["date"]=>
  int(980208000)
  ["bill-to"]=>
  &array(3) {
    ["given"]=>
    string(5) "Chris"
    ["family"]=>
    string(6) "Dumars"
    ["address"]=>
    array(4) {
      ["lines"]=>
      string(27) "458 Walkman Dr.
Suite #292
"
      ["city"]=>
      string(9) "Royal Oak"
      ["state"]=>
      string(2) "MI"
      ["postal"]=>
      int(48046)
    }
  }
  ["ship-to"]=>
  &array(3) {
    ["given"]=>
    string(5) "Chris"
    ["family"]=>
    string(6) "Dumars"
    ["address"]=>
    array(4) {
      ["lines"]=>
      string(27) "458 Walkman Dr.
Suite #292
"
      ["city"]=>
      string(9) "Royal Oak"
      ["state"]=>
      string(2) "MI"
      ["postal"]=>
      int(48046)
    }
  }
  ["product"]=>
  array(2) {
    [0]=>
    array(4) {
      ["sku"]=>
      string(6) "BL394D"
      ["quantity"]=>
      int(4)
      ["description"]=>
      string(10) "Basketball"
      ["price"]=>
      float(450)
    }
    [1]=>
    array(4) {
      ["sku"]=>
      string(7) "BL4438H"
      ["quantity"]=>
      int(1)
      ["description"]=>
      string(10) "Super Hoop"
      ["price"]=>
      float(2392)
    }
  }
  ["tax"]=>
  float(251.42)
  ["total"]=>
  float(4443.52)
  ["comments"]=>
  string(68) "Late afternoon is best. Backup contact is Nancy Billsmer @ 338-4338."
}
