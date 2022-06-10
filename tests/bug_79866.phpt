--TEST--
Test PECL bug #79866
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--INI--
precision=14
serialize_precision=-1
--FILE--
<?php
$floats = [
  "0"            => 0,
  "1"            => 1,
  "-1"           => -1,
  "2."           => 2.,
  "2.0"          => 2.0,
  "2.00"         => 2.00,
  "2.000"        => 2.000,
  "0.123456789"  => 0.123456789,
  "-0.123456789" => -0.123456789,
  "2.3e6"        => 2.3e6,
  "-2.3e6"       => -2.3e6,
  "2.3e-6"       => 2.3e-6,
  "-2.3e-6"      => -2.3e-6,
  "INF"          => INF,
  "NAN"          => NAN,
  "0.000021"     => 0.000021,
];

foreach( $floats as $idx => $float ) {
  $float = floatval($float);
  ob_start();
  echo $float;
  $native = ob_get_clean();

  $expect = "--- {$native}\n...\n";
  $got = yaml_emit($float);
  if ( $got !== $expect ) {
    echo "== FAIL! {$idx} ==\n";
    echo "expected:\n{$expect}\n";
    echo "got:{$got}\n";
  }
}
?>
--EXPECT--
