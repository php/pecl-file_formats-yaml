if (-not (Test-Path c:\build-cache)) {
    mkdir c:\build-cache
}

if ($env:PHP_VER -lt "7.4") {
    $vc =  $env:VC.toUpper()
} else {
    $vc = $env:VC
}

$bname = 'php-sdk-' + $env:BIN_SDK_VER + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "https://github.com/php/php-sdk-binary-tools/archive/$bname" -OutFile "c:\build-cache\$bname"
}
$dname0 = 'php-sdk-binary-tools-php-sdk-' + $env:BIN_SDK_VER
$dname1 = 'php-sdk-' + $env:BIN_SDK_VER
if (-not (Test-Path c:\build-cache\$dname1)) {
    7z x c:\build-cache\$bname -oc:\build-cache
    move c:\build-cache\$dname0 c:\build-cache\$dname1
}
$ts_part = ''
if ('0' -eq $env:TS) { $ts_part = '-nts' }
$bname = 'php-devel-pack-' + $env:PHP_VER + $ts_part + '-Win32-' + $vc + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "https://downloads.php.net/~windows/releases/archives/$bname" -OutFile "c:\build-cache\$bname"
    if (-not (Test-Path c:\build-cache\$bname)) {
        Invoke-WebRequest "hhttps://downloads.php.net/~windows/releases/$bname" -OutFile "c:\build-cache\$bname"
    }
}
$dname0 = 'php-' + $env:PHP_VER + '-devel-' + $vc + '-' + $env:ARCH
$dname1 = 'php-' + $env:PHP_VER + $ts_part + '-devel-' + $vc + '-' + $env:ARCH
if (-not (Test-Path c:\build-cache\$dname1)) {
    7z x c:\build-cache\$bname -oc:\build-cache
    if ($dname0 -ne $dname1) {
        move c:\build-cache\$dname0 c:\build-cache\$dname1
    }
}
$env:PHP_DEVEL_PATH = 'c:\build-cache\' + $dname1
$env:PATH = $env:PHP_DEVEL_PATH + ';' + $env:PATH

$bname = 'php-' + $env:PHP_VER + $ts_part + '-Win32-' + $vc + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "https://downloads.php.net/~windows/releases/archives/$bname" -OutFile "c:\build-cache\$bname"
    if (-not (Test-Path c:\build-cache\$bname)) {
        Invoke-WebRequest "https://downloads.php.net/~windows/releases/$bname" -OutFile "c:\build-cache\$bname"
    }
}
$dname = 'php-' + $env:PHP_VER + $ts_part + '-Win32-' + $vc + '-' + $env:ARCH
if (-not (Test-Path c:\build-cache\$dname)) {
    7z x c:\build-cache\$bname -oc:\build-cache\$dname
}
$env:PHP_PATH = 'c:\build-cache\' + $dname
$env:PATH = $env:PHP_PATH + ';' + $env:PATH

$bname = $env:DEP + '-' + $env:VC.toLower() + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "http://downloads.php.net/~windows/pecl/deps/$bname" -OutFile "c:\build-cache\$bname"
    7z x c:\build-cache\$bname -oc:\build-cache\deps
}
