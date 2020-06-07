cd c:\projects\yaml

$env:TEST_PHP_EXECUTABLE = $env:PHP_PATH + '\php.exe'
$runner = $env:TEST_PHP_EXECUTABLE
& $runner 'run-tests.php' --show-diff tests
if (-not $?) {
    throw "testing failed with errorlevel $LastExitCode"
}
