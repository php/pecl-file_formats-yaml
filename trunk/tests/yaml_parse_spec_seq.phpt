--TEST--
Yaml 1.1 Spec - seq
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
  var_dump(yaml_parse('
# Ordered sequence of nodes
Block style: !!seq
- Mercury   # Rotates - no light/dark sides.
- Venus     # Deadliest. Aptly named.
- Earth     # Mostly dirt.
- Mars      # Seems empty.
- Jupiter   # The king.
- Saturn    # Pretty.
- Uranus    # Where the sun hardly shines.
- Neptune   # Boring. No rings.
- Pluto     # You call this a planet?
Flow style: !!seq [ Mercury, Venus, Earth, Mars,      # Rocks
                    Jupiter, Saturn, Uranus, Neptune, # Gas
                    Pluto ]                           # Overrated
'));
?>
--EXPECT--
array(2) {
  ["Block style"]=>
  array(9) {
    [0]=>
    string(7) "Mercury"
    [1]=>
    string(5) "Venus"
    [2]=>
    string(5) "Earth"
    [3]=>
    string(4) "Mars"
    [4]=>
    string(7) "Jupiter"
    [5]=>
    string(6) "Saturn"
    [6]=>
    string(6) "Uranus"
    [7]=>
    string(7) "Neptune"
    [8]=>
    string(5) "Pluto"
  }
  ["Flow style"]=>
  array(9) {
    [0]=>
    string(7) "Mercury"
    [1]=>
    string(5) "Venus"
    [2]=>
    string(5) "Earth"
    [3]=>
    string(4) "Mars"
    [4]=>
    string(7) "Jupiter"
    [5]=>
    string(6) "Saturn"
    [6]=>
    string(6) "Uranus"
    [7]=>
    string(7) "Neptune"
    [8]=>
    string(5) "Pluto"
  }
}
