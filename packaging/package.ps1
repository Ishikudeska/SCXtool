param(
    [switch]$SkipPortable,
    [switch]$SkipInstaller,
    [string]$Iscc = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$dist = Join-Path $root "dist"

function Invoke-Checked([string]$exe, [string[]]$arguments) {
    & $exe @arguments
    if ($LASTEXITCODE -ne 0) { throw "$exe $($arguments -join ' ') failed with exit code $LASTEXITCODE" }
}

$identity = Get-Content (Join-Path $root "cmake\AppIdentity.cmake") -Raw
if ($identity -notmatch 'set\(APP_VERSION\s+"([^"]+)"\)') { throw "APP_VERSION not found in cmake/AppIdentity.cmake" }
$version = $Matches[1]
if ($identity -notmatch 'set\(APP_EXE_NAME\s+"([^"]+)"\)') { throw "APP_EXE_NAME not found in cmake/AppIdentity.cmake" }
$name = $Matches[1]
if ($identity -notmatch 'set\(APP_NAME\s+"([^"]+)"\)') { throw "APP_NAME not found in cmake/AppIdentity.cmake" }
$displayName = $Matches[1]
Write-Host "Packaging $displayName $version ($name)"

$cmake = (Get-Command cmake -ErrorAction SilentlyContinue)
if ($cmake) { $cmake = $cmake.Source } else { $cmake = "C:\Qt\Tools\CMake_64\bin\cmake.exe" }
if (-not (Test-Path $cmake)) { throw "cmake not found (on PATH or C:\Qt\Tools\CMake_64)" }
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.12.0\mingw_64\bin;C:\Qt\Tools\Ninja;$env:PATH"

New-Item -ItemType Directory -Force $dist | Out-Null

function Test-Deployment([string]$folder) {
    $objdump = Get-Command objdump -ErrorAction SilentlyContinue
    if (-not $objdump) { throw "objdump not found (expected in C:\Qt\Tools\mingw1310_64\bin)" }
    $system = Join-Path $env:SystemRoot "System32"
    $missing = @()
    foreach ($plugin in @("plugins\platforms\qwindows.dll", "plugins\tls\qschannelbackend.dll")) {
        if (-not (Test-Path (Join-Path $folder $plugin))) { $missing += "$plugin (plugin)" }
    }
    $binaries = Get-ChildItem $folder -Recurse -File -Include *.exe, *.dll
    $imports = @{}
    foreach ($binary in $binaries) {
        foreach ($line in (& $objdump.Source -p $binary.FullName)) {
            if ($line -match 'DLL Name:\s*(\S+)') { $imports[$Matches[1].ToLowerInvariant()] = $binary.Name }
        }
    }
    foreach ($dll in $imports.Keys) {
        if ($dll -like "api-ms-win-*" -or $dll -like "ext-ms-*") { continue }
        if (Test-Path (Join-Path $folder $dll)) { continue }
        if (Test-Path (Join-Path $system $dll)) { continue }
        $missing += "$dll (imported by $($imports[$dll]))"
    }
    if ($missing) { throw "$folder is missing:`n  $($missing -join "`n  ")" }
    Write-Host "Deployment check passed: $($binaries.Count) binaries, all imports resolved"
}

function Build-Variant([string]$preset, [string]$folder) {
    $out = Join-Path $dist $folder
    if (Test-Path $out) { Remove-Item -Recurse -Force $out }
    Push-Location $root
    try {
        Invoke-Checked $cmake @("--preset", $preset) | Out-Host
        Invoke-Checked $cmake @("--build", "--preset", $preset) | Out-Host
        Invoke-Checked $cmake @("--install", "build\$preset", "--prefix", $out) | Out-Host
    } finally {
        Pop-Location
    }
    if (-not (Test-Path (Join-Path $out "$name.exe"))) { throw "$name.exe missing from $out" }
    Test-Deployment $out | Out-Host
    return $out
}

$installed = Build-Variant "mingw-release" $name
Write-Host "Installed build: $installed"

if (-not $SkipInstaller) {
    if (-not $Iscc) {
        foreach ($candidate in @("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
                                 "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe")) {
            if (Test-Path $candidate) { $Iscc = $candidate; break }
        }
        if (-not $Iscc) {
            $onPath = Get-Command iscc -ErrorAction SilentlyContinue
            if ($onPath) { $Iscc = $onPath.Source }
        }
    }
    if ($Iscc) {
        Invoke-Checked $Iscc @("/Q", "/DAppVersion=$version", "/DAppName=$displayName", "/DAppExeName=$name",
                               "/DSourceDir=$installed", "/DOutputDir=$dist",
                               (Join-Path $PSScriptRoot "installer.iss")) | Out-Host
        Write-Host "Installer: $(Join-Path $dist "$name-$version-Setup.exe")"
    } else {
        Write-Warning "Inno Setup 6 (ISCC.exe) not found; skipping the installer. Install it from https://jrsoftware.org/isinfo.php or pass -Iscc."
    }
}

if (-not $SkipPortable) {
    $portable = Build-Variant "mingw-release-portable" "$name-Portable"
    $zip = Join-Path $dist "$name-Portable-$version.zip"
    if (Test-Path $zip) { Remove-Item -Force $zip }
    $tar = Join-Path $env:SystemRoot "System32\tar.exe"
    if (Test-Path $tar) {
        Invoke-Checked $tar @("-a", "-c", "-f", $zip, "-C", $dist, (Split-Path -Leaf $portable)) | Out-Host
    } else {
        Compress-Archive -Path $portable -DestinationPath $zip -CompressionLevel Optimal
    }
    Write-Host "Portable zip: $zip"
}
