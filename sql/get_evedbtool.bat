@@:: This prolog allows a PowerShell script to be embedded in a .CMD file.
@@:: Any non-PowerShell content must be preceeded by "@@"
@@setlocal
@@set POWERSHELL_BAT_ARGS=%*
@@if defined POWERSHELL_BAT_ARGS set POWERSHELL_BAT_ARGS=%POWERSHELL_BAT_ARGS:"=\"%
@@PowerShell -Command Invoke-Expression $('$args=@(^&{$args} %POWERSHELL_BAT_ARGS%);'+[String]::Join([char]10,$((Get-Content '%~f0') -notmatch '^^@@'))) & goto :EOF

$ErrorActionPreference = "Stop"
$version = "0.0.6"
$file = "evedbtool.exe"
$expectedSha256 = "090122a4155eed5305fea14ce2c0add87dd478ca3ca0a776dd99e6018df08a7f"
$download = "https://github.com/EvEmu-Project/EVEDBTool/releases/download/$version/$file"
$target = Join-Path (Get-Location) $file
$temporary = "$target.$([Guid]::NewGuid().ToString('N')).tmp"

try {
    Invoke-WebRequest -UseBasicParsing -Uri $download -OutFile $temporary
    $actualSha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $temporary).Hash
    if ($actualSha256 -ne $expectedSha256) {
        throw "EVEDBTool checksum mismatch"
    }
    Move-Item -Force -LiteralPath $temporary -Destination $target
}
catch {
    Remove-Item -Force -ErrorAction SilentlyContinue -LiteralPath $temporary
    throw
}

