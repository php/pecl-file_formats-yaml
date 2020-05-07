cd c:\projects\yaml
echo "" | Out-File -Encoding "ASCII" task.bat
echo "call phpize 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "call configure --with-php-build=c:\build-cache\deps --with-yaml --enable-debug-pack 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "nmake /nologo 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "exit %errorlevel%" | Out-File -Encoding "ASCII" -Append task.bat
$here = (Get-Item -Path "." -Verbose).FullName
$runner = 'c:\build-cache\php-sdk-' + $env:BIN_SDK_VER + '\phpsdk' + '-' + $env:VC + '-' + $env:ARCH + '.bat'
$task = $here + '\task.bat'
& $runner -t $task
if (-not $?) {
    throw "building failed with errorlevel $LastExitCode"
}
$dname = ''
if ('x64' -eq  $env:ARCH) { $dname = $dname + 'x64\'}
$dname = $dname + 'Release';
if ('1' -eq $env:TS) { $dname = $dname + '_TS'}
copy $dname\php_yaml.dll $env:PHP_PATH\ext\php_yaml.dll

New-Item -Path $env:PHP_PATH\php.ini -Force
Add-Content -Path $env:PHP_PATH\php.ini -Value 'extension_dir=ext'
Add-Content -Path $env:PHP_PATH\php.ini -Value 'extension=php_yaml.dll'
