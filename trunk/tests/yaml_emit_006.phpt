--TEST--
yaml_emit - ini settings
--SKIPIF--
<?php
if(!extension_loaded('yaml')) die('skip yaml n/a');
?>
--INI--
yaml.output_canonical=0
yaml.output_indent=2
yaml.output_width=80
--FILE--
<?php
$addr = array(
    "given" => "Chris",
    "family"=> "Dumars",
    "address"=> array(
        "lines"=> "458 Walkman Dr.
        Suite #292",
        "city"=> "Royal Oak",
        "state"=> "MI",
        "postal"=> 48046,
      ),
  );
$invoice = array (
    "invoice"=> 34843,
    "date"=> 980208000,
    "bill-to"=> $addr,
    "ship-to"=> $addr,
    "product"=> array(
        array(
            "sku"=> "BL394D",
            "quantity"=> 4,
            "description"=> "Basketball",
            "price"=> 450,
          ),
        array(
            "sku"=> "BL4438H",
            "quantity"=> 1,
            "description"=> "Super Hoop",
            "price"=> 2392,
          ),
      ),
    "tax"=> 251.42,
    "total"=> 4443.52,
    "comments"=> "Late afternoon is best. Backup contact is Nancy Billsmer @ 338-4338.",
  );
var_dump(yaml_emit($invoice));

ini_set("yaml.output_canonical", "1");
ini_set("yaml.output_indent", "4");
ini_set("yaml.output_width", "40");
echo "== CANONICAL ==\n";
var_dump(yaml_emit($invoice));

?>
--EXPECT--
string(628) "---
invoice: 34843
date: 980208000
bill-to:
  given: Chris
  family: Dumars
  address:
    lines: |-
      458 Walkman Dr.
              Suite #292
    city: Royal Oak
    state: MI
    postal: 48046
ship-to:
  given: Chris
  family: Dumars
  address:
    lines: |-
      458 Walkman Dr.
              Suite #292
    city: Royal Oak
    state: MI
    postal: 48046
product:
- sku: BL394D
  quantity: 4
  description: Basketball
  price: 450
- sku: BL4438H
  quantity: 1
  description: Super Hoop
  price: 2392
tax: 251.420000
total: 4443.520000
comments: Late afternoon is best. Backup contact is Nancy Billsmer @ 338-4338.
...
"
== CANONICAL ==
string(1830) "---
!!map {
    ? !!str "invoice"
    : !!int "34843",
    ? !!str "date"
    : !!int "980208000",
    ? !!str "bill-to"
    : !!map {
        ? !!str "given"
        : !!str "Chris",
        ? !!str "family"
        : !!str "Dumars",
        ? !!str "address"
        : !!map {
            ? !!str "lines"
            : !!str "458 Walkman Dr.\n        Suite
                #292",
            ? !!str "city"
            : !!str "Royal Oak",
            ? !!str "state"
            : !!str "MI",
            ? !!str "postal"
            : !!int "48046",
        },
    },
    ? !!str "ship-to"
    : !!map {
        ? !!str "given"
        : !!str "Chris",
        ? !!str "family"
        : !!str "Dumars",
        ? !!str "address"
        : !!map {
            ? !!str "lines"
            : !!str "458 Walkman Dr.\n        Suite
                #292",
            ? !!str "city"
            : !!str "Royal Oak",
            ? !!str "state"
            : !!str "MI",
            ? !!str "postal"
            : !!int "48046",
        },
    },
    ? !!str "product"
    : !!seq [
        !!map {
            ? !!str "sku"
            : !!str "BL394D",
            ? !!str "quantity"
            : !!int "4",
            ? !!str "description"
            : !!str "Basketball",
            ? !!str "price"
            : !!int "450",
        },
        !!map {
            ? !!str "sku"
            : !!str "BL4438H",
            ? !!str "quantity"
            : !!int "1",
            ? !!str "description"
            : !!str "Super Hoop",
            ? !!str "price"
            : !!int "2392",
        },
    ],
    ? !!str "tax"
    : !!float "251.420000",
    ? !!str "total"
    : !!float "4443.520000",
    ? !!str "comments"
    : !!str "Late afternoon is best. Backup
        contact is Nancy Billsmer @ 338-4338.",
}
...
"
