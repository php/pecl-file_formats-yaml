--TEST--
Test PECL bug #77720
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
$key_laughs = <<<YAML
- &a ["lol","lol","lol","lol","lol","lol","lol","lol","lol"]
- ? &b [{*a:1},{*a:1},{*a:1},{*a:1},{*a:1},{*a:1},{*a:1},{*a:1},{*a:}] : "foo"
- ? &c [{*b:1},{*b:1},{*b:1},{*b:1},{*b:1},{*b:1},{*b:1},{*b:1},{*b:}] : "foo"
- ? &d [{*c:1},{*c:1},{*c:1},{*c:1},{*c:1},{*c:1},{*c:1},{*c:1},{*c:}] : "foo"
- ? &e [{*d:1},{*d:1},{*d:1},{*d:1},{*d:1},{*d:1},{*d:1},{*d:1},{*d:}] : "foo"
- ? &f [{*e:1},{*e:1},{*e:1},{*e:1},{*e:1},{*e:1},{*e:1},{*e:1},{*e:}] : "foo"
- ? &g [{*f:1},{*f:1},{*f:1},{*f:1},{*f:1},{*f:1},{*f:1},{*f:1},{*f:}] : "foo"
- ? &h [{*g:1},{*g:1},{*g:1},{*g:1},{*g:1},{*g:1},{*g:1},{*g:1},{*g:}] : "foo"
- ? &i [{*h:1},{*h:1},{*h:1},{*h:1},{*h:1},{*h:1},{*h:1},{*h:1},{*h:}] : "foo"
- ? &j [{*i:1},{*i:1},{*i:1},{*i:1},{*i:1},{*i:1},{*i:1},{*i:1},{*i:}] : "foo"
- ? &k [{*j:1},{*j:1},{*j:1},{*j:1},{*j:1},{*j:1},{*j:1},{*j:1},{*j:}] : "foo"
YAML;
var_dump(yaml_parse($key_laughs));
?>
--EXPECTF--

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 2, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 3, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 4, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 5, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 6, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 7, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 8, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 9, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 10, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 2) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 73) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 11, column 79) in %s/bug_77720.php on line 15

Warning: yaml_parse(): Illegal offset type array (line 12, column 1) in %s/bug_77720.php on line 15
array(11) {
  [0]=>
  array(9) {
    [0]=>
    string(3) "lol"
    [1]=>
    string(3) "lol"
    [2]=>
    string(3) "lol"
    [3]=>
    string(3) "lol"
    [4]=>
    string(3) "lol"
    [5]=>
    string(3) "lol"
    [6]=>
    string(3) "lol"
    [7]=>
    string(3) "lol"
    [8]=>
    string(3) "lol"
  }
  [1]=>
  array(0) {
  }
  [2]=>
  array(0) {
  }
  [3]=>
  array(0) {
  }
  [4]=>
  array(0) {
  }
  [5]=>
  array(0) {
  }
  [6]=>
  array(0) {
  }
  [7]=>
  array(0) {
  }
  [8]=>
  array(0) {
  }
  [9]=>
  array(0) {
  }
  [10]=>
  array(0) {
  }
}
