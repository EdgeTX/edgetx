# setup-msvc.ps1
# Runs vcvarsall.bat and exports only the new/changed environment
# variables to GitHub Actions, avoiding clobbering existing ones.

param(
    [string]$Arch = "x64"
)

# Find VS installation
$vsWhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath  = & $vsWhere -latest -property installationPath
$vcvars  = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvars)) {
    Write-Error "vcvarsall.bat not found at: $vcvars"
    exit 1
}

# Snapshot environment before
$before = @{}
Get-ChildItem env: | ForEach-Object { $before[$_.Name] = $_.Value }

# Run vcvarsall.bat and import its environment into this process
cmd /c "`"$vcvars`" $Arch && set" | ForEach-Object {
    if ($_ -match '^([^=]+)=(.*)$') {
        [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2])
    }
}

# Prefer the clang bundled with Visual Studio. vcvarsall appends it to the end
# of PATH, so a standalone LLVM preinstalled on the runner would be found first.
# The MSVC STL refuses clang versions older than the one it was released with
# (error STL1000), so the bundled clang is the one guaranteed to match.
$vsLlvm = Join-Path $vsPath "VC\Tools\Llvm\x64\bin"
if (Test-Path (Join-Path $vsLlvm "clang++.exe")) {
    $env:PATH = "$vsLlvm;$env:PATH"
    Write-Host "Using Visual Studio clang from: $vsLlvm"
    & (Join-Path $vsLlvm "clang++.exe") --version
} else {
    Write-Warning "Visual Studio clang not found at $vsLlvm; using clang from PATH"
}

# Export only changed/new variables to GitHub Actions
$count = 0
Get-ChildItem env: | ForEach-Object {
    if ($before[$_.Name] -ne $_.Value) {
        "$($_.Name)=$($_.Value)" | Out-File -Append -Encoding utf8 $env:GITHUB_ENV
        $count++
    }
}

Write-Host "Exported $count environment variables to GITHUB_ENV"
