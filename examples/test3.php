<?php
extension_loaded('yaml') || dl('yaml.so') || exit(1);

$br = defined('PHP_EOL') ? PHP_EOL : (strncasecmp(PHP_OS, 'WIN', 3) ? "\n" : "\r\n");
$id = ($_SERVER['argc'] > 1) ? intval($_SERVER['argv'][1]) : 1;
$yaml = sprintf('example%02d.yaml', $id);
if (!file_exists($yaml)) {
    echo "$yaml not exists.$br";
    exit(1);
}
ini_set('yaml.decode_binary', 1);
if (function_exists('date_create')) {
    ini_set('yaml.decode_timestamp', 2);
} else {
    ini_set('yaml.decode_timestamp', 1);
}
$data = file_get_contents($yaml);

echo "+----------+$br";
echo "|   YAML   |$br";
echo "+----------+$br";
echo $data;
echo $br;
echo "+----------+$br";
echo "|  RESULT  |$br";
echo "+----------+$br";
$callbacks = array(
    'php_unique_array' => 'array_unique',
    'php_serialized_var' => 'unserialize',
    'tag:clarkevans.com,2002:invoice' => 'serialize',
);
var_dump(yaml_parse($data, -1, $ndocs, $callbacks));
