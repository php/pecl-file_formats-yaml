--TEST--
Test PECL bug #79567
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$data = [
    'audioEnabled' => [
        0 => 132317787432502136,
        1 => 0,
    ],
];
$yaml = yaml_emit($data);
$result = yaml_parse($yaml);
print $result == $data ? "Yes!\n" : "No...\n";
?>
--EXPECT--
Yes!
