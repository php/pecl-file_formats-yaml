--TEST--
yaml_parse
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
yaml.decode_timestamp=1
date.timezone=GMT
--FILE--
<?php
  var_dump(yaml_parse('
--- !<tag:clarkevans.com,2002:invoice>
invoice: 34843
date   : 2001-01-23
bill-to: &id001
    given  : Chris
    family : Dumars
    address:
        lines: |
            458 Walkman Dr.
            Suite #292
        city    : Royal Oak
        state   : MI
        postal  : 48046
ship-to: *id001
product:
    - sku         : BL394D
      quantity    : 4
      description : Basketball
      price       : 450.00
    - sku         : BL4438H
      quantity    : 1
      description : Super Hoop
      price       : 2392.00
tax  : 251.42
total: 4443.52
comments:
    Late afternoon is best.
    Backup contact is Nancy
    Billsmer @ 338-4338.
'));
  var_dump(yaml_parse('
---
Time: 2001-11-23 15:01:42 -5
User: ed
Warning:
  This is an error message
  for the log file
---
Time: 2001-11-23 15:02:31 -5
User: ed
Warning:
  A slightly different error
  message.
---
Date: 2001-11-23 15:03:17 -5
User: ed
Fatal:
  Unknown variable "bar"
Stack:
  - file: TopClass.py
    line: 23
    code: |
      x = MoreObject("345\n")
  - file: MoreClass.py
    line: 58
    code: |-
      foo = bar
', -1));
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
array(3) {
  [0]=>
  array(3) {
    ["Time"]=>
    int(1006545702)
    ["User"]=>
    string(2) "ed"
    ["Warning"]=>
    string(41) "This is an error message for the log file"
  }
  [1]=>
  array(3) {
    ["Time"]=>
    int(1006545751)
    ["User"]=>
    string(2) "ed"
    ["Warning"]=>
    string(35) "A slightly different error message."
  }
  [2]=>
  array(4) {
    ["Date"]=>
    int(1006545797)
    ["User"]=>
    string(2) "ed"
    ["Fatal"]=>
    string(22) "Unknown variable "bar""
    ["Stack"]=>
    array(2) {
      [0]=>
      array(3) {
        ["file"]=>
        string(11) "TopClass.py"
        ["line"]=>
        int(23)
        ["code"]=>
        string(24) "x = MoreObject("345\n")
"
      }
      [1]=>
      array(3) {
        ["file"]=>
        string(12) "MoreClass.py"
        ["line"]=>
        int(58)
        ["code"]=>
        string(9) "foo = bar"
      }
    }
  }
}
