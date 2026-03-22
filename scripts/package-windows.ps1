$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $PSScriptRoot
$BuildDir = if ($args.Length -gt 0) { $args[0] } else { Join-Path $RootDir "build" }
$OutDir = if ($args.Length -gt 1) { $args[1] } else { Join-Path $RootDir "out\windows" }

$PluginCandidates = @(
    (Join-Path $BuildDir "HeadturnerHat_artefacts\Release\VST3\Headturner Hat.vst3"),
    (Join-Path $BuildDir "HeadturnerHat_artefacts\VST3\Headturner Hat.vst3")
)
$PluginBundle = $PluginCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
$IssSource = Join-Path $RootDir "packaging\windows\HeadturnerHat.iss"
$WorkDir = Join-Path $OutDir "installer-work"
$InnoCommand = Get-Command iscc -ErrorAction SilentlyContinue
$InnoPath = if ($InnoCommand) {
    $InnoCommand.Source
} else {
    $InnoFallback = Join-Path ${env:ProgramFiles(x86)} "Inno Setup 6\ISCC.exe"

    if (Test-Path $InnoFallback) {
        $InnoFallback
    } else {
        throw "Could not find ISCC.exe"
    }
}

if (-not $PluginBundle) {
    throw "Could not find Headturner Hat.vst3 in $BuildDir"
}

Remove-Item -Recurse -Force $WorkDir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null
Copy-Item -Recurse -Force $PluginBundle $WorkDir
Copy-Item -Force $IssSource $WorkDir

Push-Location $WorkDir
& $InnoPath HeadturnerHat.iss
Pop-Location

$Installer = Join-Path $WorkDir "HeadturnerHat-Windows-Installer.exe"
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
Copy-Item -Force $Installer $OutDir

Write-Output (Join-Path $OutDir "HeadturnerHat-Windows-Installer.exe")
