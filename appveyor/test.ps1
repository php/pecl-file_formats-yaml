cd c:\projects\yaml

$env:TEST_PHP_EXECUTABLE = $env:PHP_PATH + '\php.exe'
$runner = $env:TEST_PHP_EXECUTABLE
$run_tests= $env:PHP_TEST_PATH + '\run-test.php'
& $runner $run_tests --show-diff tests
if (-not $?) {
    throw "testing failed with errorlevel $LastExitCode"
}
