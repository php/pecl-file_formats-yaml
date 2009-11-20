--TEST--
yaml_emit Scalars
--SKIPIF--
<?php if(!extension_loaded('yaml')) die('skip yaml n/a'); ?>
--FILE--
<?php
var_dump(yaml_emit(null));
var_dump(yaml_emit(true));
var_dump(yaml_emit(false));
var_dump(yaml_emit(10));
var_dump(yaml_emit(-10));
var_dump(yaml_emit(123.456));
var_dump(yaml_emit(-123.456));
var_dump(yaml_emit("yes"));
var_dump(yaml_emit("no"));
var_dump(yaml_emit("~"));
var_dump(yaml_emit("-"));
var_dump(yaml_emit("'"));
var_dump(yaml_emit('"'));
var_dump(yaml_emit("I\\xF1t\\xEBrn\\xE2ti\\xF4n\\xE0liz\\xE6ti\\xF8n"));
var_dump(yaml_emit("# looks like a comment"));
var_dump(yaml_emit("@looks_like_a_ref"));
var_dump(yaml_emit("&looks_like_a_alias"));
var_dump(yaml_emit("!!str"));
var_dump(yaml_emit("%TAG ! tag:looks.like.one,999:"));
var_dump(yaml_emit("!something"));
var_dump(yaml_emit("Hello world!"));
var_dump(yaml_emit("This is a string with\nan embedded newline."));
$str = <<<EOD
This string was made with a here doc.

It contains embedded newlines.
  		It also has some embedded tabs.

Here are some symbols:
`~!@#$%^&*()_-+={}[]|\:";'<>,.?/

These are extended characters: Iñtërnâtiônàlizætiøn


EOD;
var_dump(yaml_emit($str));
?>
--EXPECT--
string(10) "--- ~
...
"
string(13) "--- true
...
"
string(14) "--- false
...
"
string(11) "--- 10
...
"
string(12) "--- -10
...
"
string(19) "--- 123.456000
...
"
string(20) "--- -123.456000
...
"
string(14) "--- "yes"
...
"
string(13) "--- "no"
...
"
string(12) "--- "~"
...
"
string(12) "--- '-'
...
"
string(13) "--- ''''
...
"
string(12) "--- '"'
...
"
string(50) "--- I\xF1t\xEBrn\xE2ti\xF4n\xE0liz\xE6ti\xF8n
...
"
string(33) "--- '# looks like a comment'
...
"
string(28) "--- '@looks_like_a_ref'
...
"
string(30) "--- '&looks_like_a_alias'
...
"
string(16) "--- '!!str'
...
"
string(41) "--- '%TAG ! tag:looks.like.one,999:'
...
"
string(21) "--- '!something'
...
"
string(21) "--- Hello world!
...
"
string(58) "--- |-
  This is a string with
  an embedded newline.
...
"
string(267) "--- "This string was made with a here doc.\n\nIt contains embedded newlines.\n  \t\tIt
  also has some embedded tabs.\n\nHere are some symbols:\n`~!@#$%^&*()_-+={}[]|\\:\";'<>,.?/\n\nThese
  are extended characters: I\xF1t\xEBrn\xE2ti\xF4n\xE0liz\xE6ti\xF8n\n\n"
...
"
