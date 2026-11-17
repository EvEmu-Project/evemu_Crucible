# Generate a 50x50 JPEG in the ImageServer imageDir/Character directory.
# This works with the file-first EVEmu PR #25 update, not older embedded-placeholder builds.
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string]$ImageDir,
    [ValidateRange(3000000,3999999)] [int]$AgentId = 3011918,
    [string]$SourceJpeg,
    [switch]$GenericFallback,
    [switch]$Overwrite
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$dir = Join-Path $ImageDir 'Character'
New-Item -ItemType Directory -Force -Path $dir | Out-Null
$name = if ($GenericFallback) { 'agent_fallback_50.jpg' } else { "{0}_50.jpg" -f $AgentId }
$target = Join-Path $dir $name
if (Test-Path -LiteralPath $target) {
    if (-not $Overwrite) { throw "File already exists: $target. Use -Overwrite to back up and replace it." }
    $backup = '{0}.backup-{1}' -f $target, (Get-Date -Format 'yyyyMMdd-HHmmss')
    Copy-Item -LiteralPath $target -Destination $backup
    Write-Host "Backed up: $backup"
}
$tmp = Join-Path $dir ('.tmp-{0}.jpg' -f [guid]::NewGuid().ToString('N'))
$bmp = $null
$g = $null
$source = $null
try {
    $bmp = [System.Drawing.Bitmap]::new(50, 50, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    if ($SourceJpeg) {
        $source = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $SourceJpeg).Path)
        $g.DrawImage($source, 0, 0, 50, 50)
    } else {
        $g.Clear([System.Drawing.Color]::FromArgb(61, 72, 84))
        $brush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(158, 169, 181))
        try {
            $g.FillEllipse($brush, 17, 10, 16, 16)
            $g.FillEllipse($brush, 8, 29, 34, 28)
        } finally { $brush.Dispose() }
    }
    $g.Dispose()
    $g = $null
    $bmp.Save($tmp, [System.Drawing.Imaging.ImageFormat]::Jpeg)
} finally {
    if ($source) { $source.Dispose() }
    if ($g) { $g.Dispose() }
    if ($bmp) { $bmp.Dispose() }
}
try {
    $data = [System.IO.File]::ReadAllBytes($tmp)
    if ($data.Length -lt 100 -or $data[0] -ne 255 -or $data[1] -ne 216 -or
        $data[$data.Length-2] -ne 255 -or $data[$data.Length-1] -ne 217) {
        throw 'JPEG marker check failed.'
    }
    $test = [System.Drawing.Image]::FromFile($tmp)
    try {
        if ($test.Width -ne 50 -or $test.Height -ne 50) { throw 'Portrait is not 50x50.' }
    } finally { $test.Dispose() }
    Move-Item -LiteralPath $tmp -Destination $target -Force
    Write-Host ("Created {0} ({1} bytes). Now test GET /Character/{2}_50.jpg" -f $target, $data.Length, $AgentId)
} catch {
    if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force }
    throw
}
