<?php
extension_loaded('yaml') || dl('yaml.so') || exit(1);
extension_loaded('syck') || @dl('syck.so') || exit(1);

$br = defined('PHP_EOL') ? PHP_EOL : (strncasecmp(PHP_OS, 'WIN', 3) ? "\n" : "\r\n");
$id = ($_SERVER['argc'] > 1) ? intval($_SERVER['argv'][1]) : 1;
$yaml = sprintf('example%02d.yaml', $id);
if (!file_exists($yaml)) {
    echo "$yaml not exists.$br";
    exit(1);
}
$data = file_get_contents($yaml);

echo "+----------+$br";
echo "|   YAML   |$br";
echo "+----------+$br";
echo $data;
echo $br;
echo "+----------+$br";
echo "| LibYAML  |$br";
echo "+----------+$br";
if (function_exists('date_create')) {
    eval('class MyDateTime extends DateTime {
        static public function create($timestamp) {
            return new MyDateTime($timestamp);
        }
        public function __toString() {
            return $this->format(self::ISO8601);
        }
    }');
    var_dump($y = yaml_parse($data, 0, $ndocs,
        array('tag:yaml.org,2002:timestamp' => array('MyDateTime', 'create'))));
} else {
    var_dump($y = yaml_parse($data));
}
echo $br;
echo "+----------+$br";
echo "|   Syck   |$br";
echo "+----------+$br";
var_dump($s = syck_load($data));
$s = array();
echo $br;
echo "+----------+$br";
echo "|   diff   |$br";
echo "+----------+$br";
if (gettype($y) != gettype($s)) {
    var_dump(false);
} elseif (is_array($y)) {
    var_dump(array_diff($y, $s));
} else {
    var_dump($y == $s);
}
