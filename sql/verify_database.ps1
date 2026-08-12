[CmdletBinding()]
param(
    [string]$ProjectRoot = '',
    [string]$ToolsRoot = '',
    [string]$WorkRoot = '',
    [ValidateRange(1024, 65535)]
    [int]$Port = 13306,
    [switch]$KeepData
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = Join-Path $PSScriptRoot '..'
}
if ([string]::IsNullOrWhiteSpace($ToolsRoot)) {
    $ToolsRoot = Join-Path $PSScriptRoot '..\..\Tools'
}

function Resolve-Directory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "$Description was not found."
    }

    return (Resolve-Path -LiteralPath $Path).Path
}

function Resolve-File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description was not found."
    }

    return (Resolve-Path -LiteralPath $Path).Path
}

function Convert-ToForwardSlash {
    param([Parameter(Mandatory = $true)][string]$Path)

    return $Path.Replace('\', '/')
}

function Write-Utf8File {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Content
    )

    $encoding = New-Object System.Text.UTF8Encoding
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Invoke-NativeCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [string[]]$Arguments = @()
    )

    $previousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $output = (& $Path @Arguments 2>&1 | Out-String).Trim()
    }
    finally {
        $ErrorActionPreference = $previousPreference
    }
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) {
        $name = [System.IO.Path]::GetFileName($Path)
        if ($output.Length -gt 2000) {
            $output = $output.Substring($output.Length - 2000)
        }
        throw "$name failed with exit code $exitCode. $output"
    }

    return $output
}

function New-RandomHex {
    param([Parameter(Mandatory = $true)][int]$ByteCount)

    $bytes = New-Object byte[] $ByteCount
    $generator = [System.Security.Cryptography.RandomNumberGenerator]::Create()
    try {
        $generator.GetBytes($bytes)
    }
    finally {
        $generator.Dispose()
    }

    $hex = ([System.BitConverter]::ToString($bytes)).Replace('-', '')
    return $hex.ToLowerInvariant()
}

function Invoke-Database {
    param(
        [Parameter(Mandatory = $true)]
        [string]$OptionFile,
        [Parameter(Mandatory = $true)]
        [string]$Database,
        [Parameter(Mandatory = $true)]
        [string]$Sql,
        [switch]$Scalar
    )

    $arguments = @(
        "--defaults-extra-file=$OptionFile",
        '--protocol=tcp',
        '--skip-ssl',
        '--skip-auto-rehash',
        "--database=$Database",
        '--batch',
        '--raw'
    )
    if ($Scalar) {
        $arguments += '--skip-column-names'
    }
    $arguments += "--execute=$Sql"

    $result = Invoke-NativeCommand $mariadb $arguments
    if ($Scalar) {
        return $result.Trim()
    }

    return $result
}

function Invoke-DatabaseScript {
    param(
        [Parameter(Mandatory = $true)]
        [string]$OptionFile,
        [Parameter(Mandatory = $true)]
        [string]$Database,
        [Parameter(Mandatory = $true)]
        [string]$ScriptPath
    )

    $sourceDirectory = Split-Path -Parent $ScriptPath
    $sourceName = Split-Path -Leaf $ScriptPath
    Push-Location $sourceDirectory
    try {
        $arguments = @(
            "--defaults-extra-file=$OptionFile",
            '--protocol=tcp',
            '--skip-ssl',
            '--skip-auto-rehash',
            "--database=$Database",
            "--execute=source $sourceName"
        )
        $null = Invoke-NativeCommand $mariadb $arguments
    }
    finally {
        Pop-Location
    }
}

function Invoke-EveDbTool {
    param([Parameter(Mandatory = $true)][string]$Command)

    Push-Location $workRoot
    try {
        return (Invoke-NativeCommand $evedbtool @($Command))
    }
    finally {
        Pop-Location
    }
}

function Get-DatabaseScalar {
    param(
        [Parameter(Mandatory = $true)]
        [string]$OptionFile,
        [Parameter(Mandatory = $true)]
        [string]$Database,
        [Parameter(Mandatory = $true)]
        [string]$Sql
    )

    $value = Invoke-Database $OptionFile $Database $Sql -Scalar
    if ([string]::IsNullOrWhiteSpace($value)) {
        throw 'Database query returned no scalar value.'
    }

    return $value.Trim()
}

$scriptWorkspaceRoot = Resolve-Directory (
    Join-Path $PSScriptRoot '..\..') 'Script workspace root'
$ProjectRoot = Resolve-Directory $ProjectRoot 'Project root'
$ToolsRoot = Resolve-Directory $ToolsRoot 'Tools directory'
$workspaceRoot = Resolve-Directory (
    Split-Path -Parent $ProjectRoot) 'Workspace root'
if (-not $workspaceRoot.Equals(
        $scriptWorkspaceRoot,
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'ProjectRoot must be inside the script workspace.'
}
$workspacePathPrefix = $scriptWorkspaceRoot.TrimEnd('\') + '\'
if (-not $ToolsRoot.StartsWith(
        $workspacePathPrefix,
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'ToolsRoot must be inside the script workspace.'
}
$sqlRoot = Resolve-Directory (Join-Path $ProjectRoot 'sql') 'SQL directory'
$baseRoot = Resolve-Directory (Join-Path $sqlRoot 'base') 'Base SQL directory'
$migrationRoot = Resolve-Directory (
    Join-Path $sqlRoot 'migrations') 'Migration directory'
$dungeonRoot = Resolve-Directory (
    Join-Path $sqlRoot 'dungeons') 'Dungeon directory'

$mariaRoot = Join-Path $ToolsRoot 'mariadb-12.3.2\mariadb-12.3.2-winx64\bin'
$mariadbd = Resolve-File (Join-Path $mariaRoot 'mariadbd.exe') 'MariaDB server'
$mariadb = Resolve-File (Join-Path $mariaRoot 'mariadb.exe') 'MariaDB client'
$mariadbDump = Resolve-File (
    Join-Path $mariaRoot 'mariadb-dump.exe') 'MariaDB dump tool'
$installDb = Resolve-File (
    Join-Path $mariaRoot 'mariadb-install-db.exe') 'MariaDB initializer'
$evedbtool = Resolve-File (
    Join-Path $ToolsRoot 'evedbtool-0.0.6\evedbtool.exe') 'EVEDBTool'

$runStamp = Get-Date -Format 'yyyyMMddHHmmss'
if ([string]::IsNullOrWhiteSpace($WorkRoot)) {
    $WorkRoot = Join-Path (Join-Path $ProjectRoot 'sql') (
        "evemu-db-verify-$runStamp-$PID")
}

if (-not [System.IO.Path]::IsPathRooted($WorkRoot)) {
    $WorkRoot = Join-Path (Get-Location).Path $WorkRoot
}
$WorkRoot = [System.IO.Path]::GetFullPath($WorkRoot)
$workspacePrefix = $workspaceRoot.TrimEnd('\') + '\'
if (-not $WorkRoot.StartsWith(
        $workspacePrefix,
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'WorkRoot must be inside the workspace.'
}
if ($WorkRoot.Equals(
        $workspaceRoot.TrimEnd('\'),
        [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'WorkRoot cannot be the workspace root.'
}

$workRootCreated = $false
$serverProcess = $null
$rootOptionFile = $null
$appOptionFile = $null
$configPath = $null
$userSetupPath = $null
$lastStartupError = ''
$secretFiles = New-Object System.Collections.Generic.List[string]

try {
    $workParent = Split-Path -Parent $WorkRoot
    if (-not (Test-Path -LiteralPath $workParent -PathType Container)) {
        throw 'The parent directory for WorkRoot does not exist.'
    }
    if (Test-Path -LiteralPath $WorkRoot) {
        throw 'WorkRoot already exists; refusing to reuse it.'
    }

    New-Item -ItemType Directory -Path $WorkRoot | Out-Null
    $workRoot = (Resolve-Path -LiteralPath $WorkRoot).Path
    $workRootCreated = $true

    $dataRoot = Join-Path $workRoot 'data'
    New-Item -ItemType Directory -Path $dataRoot | Out-Null
    $rootOptionFile = Join-Path $workRoot 'root.cnf'
    $appOptionFile = Join-Path $workRoot 'app.cnf'
    $configPath = Join-Path $workRoot 'evedb.yaml'
    $userSetupPath = Join-Path $workRoot 'create-user.sql'
    $serverLog = Join-Path $workRoot 'mariadb-error.log'
    $serverOutput = Join-Path $workRoot 'mariadb-output.log'
    $serverError = Join-Path $workRoot 'mariadb-stderr.log'
    $backupPath = Join-Path $workRoot 'database-backup.sql'

    $database = "evemu_verify_$runStamp$PID"
    $restoreDatabase = "${database}_restore"
    $dbUser = "evemu_verify_user_$PID"
    $dbPassword = New-RandomHex 24

    $secretFiles.Add($rootOptionFile)
    $secretFiles.Add($appOptionFile)
    $secretFiles.Add($configPath)
    $secretFiles.Add($userSetupPath)

    Write-Utf8File $rootOptionFile @"
[client]
host=127.0.0.1
port=$Port
user=root
"@

    $null = Invoke-NativeCommand $installDb @(
        "--datadir=$dataRoot",
        "--port=$Port",
        '--default-user',
        '--silent'
    )

    $serverArguments = @(
        "--datadir=$dataRoot",
        "--port=$Port",
        '--bind-address=127.0.0.1',
        '--skip-name-resolve',
        '--max-connections=32',
        '--max-allowed-packet=16M',
        "--log-error=$serverLog",
        '--console'
    )
    $serverProcess = Start-Process `
        -FilePath $mariadbd `
        -ArgumentList $serverArguments `
        -RedirectStandardOutput $serverOutput `
        -RedirectStandardError $serverError `
        -PassThru `
        -WindowStyle Hidden

    $startupTimeoutSeconds = 60
    $connected = $false
    for ($attempt = 0; $attempt -lt $startupTimeoutSeconds; $attempt++) {
        if ($serverProcess.HasExited) {
            throw 'MariaDB exited before accepting connections.'
        }

        try {
            $probe = Get-DatabaseScalar $rootOptionFile 'mysql' 'SELECT 1'
            if ($probe -eq '1') {
                $connected = $true
                break
            }
        }
        catch {
            $lastStartupError = $_.Exception.Message
        }

        Start-Sleep -Seconds 1
    }
    if (-not $connected) {
        throw 'MariaDB did not accept a connection within the startup limit.'
    }

    $userSetupSql = @"
CREATE DATABASE $database;
CREATE DATABASE $restoreDatabase;
CREATE USER '$dbUser'@'127.0.0.1' IDENTIFIED BY '$dbPassword';
GRANT ALL PRIVILEGES ON $database.* TO '$dbUser'@'127.0.0.1';
GRANT ALL PRIVILEGES ON $restoreDatabase.* TO '$dbUser'@'127.0.0.1';
FLUSH PRIVILEGES;
"@
    Write-Utf8File $userSetupPath $userSetupSql
    Invoke-DatabaseScript $rootOptionFile 'mysql' $userSetupPath

    Write-Utf8File $appOptionFile @"
[client]
host=127.0.0.1
port=$Port
user=$dbUser
password=$dbPassword
"@

    $basePath = Convert-ToForwardSlash $baseRoot
    $migrationPath = Convert-ToForwardSlash $migrationRoot
    $dungeonPath = Convert-ToForwardSlash $dungeonRoot
    Write-Utf8File $configPath @"
base-dir: $basePath
db-database: $database
db-host: 127.0.0.1
db-pass: $dbPassword
db-port: "$Port"
db-user: $dbUser
dungeons-dir: $dungeonPath
log-level: Info
migrations-dir: $migrationPath
"@

    $null = Invoke-EveDbTool 'install'

    $tableSql = "SELECT COUNT(*) FROM information_schema.tables " +
        "WHERE table_schema = '$database'"
    $sourceTableCount = [int](Get-DatabaseScalar `
        $appOptionFile $database $tableSql)
    if ($sourceTableCount -lt 1) {
        throw 'EVEDBTool created no tables.'
    }

    $kdfSql = "SELECT COUNT(*) FROM information_schema.columns " +
        "WHERE table_schema = '$database' AND table_name = 'account' " +
        "AND column_name = 'passwordKdf'"
    if ([int](Get-DatabaseScalar $appOptionFile $database $kdfSql) -ne 1) {
        throw 'The password-KDF migration was not applied.'
    }

    $dumpArguments = @(
        "--defaults-extra-file=$appOptionFile",
        '--protocol=tcp',
        '--skip-ssl',
        '--single-transaction',
        '--routines',
        '--events',
        '--triggers',
        "--result-file=$backupPath",
        $database
    )
    $null = Invoke-NativeCommand $mariadbDump $dumpArguments
    $backupBytes = (Get-Item -LiteralPath $backupPath).Length
    if ($backupBytes -lt 1) {
        throw 'The logical database backup is empty.'
    }

    $restoreTableSql = "SELECT COUNT(*) FROM information_schema.tables " +
        "WHERE table_schema = '$restoreDatabase'"
    Invoke-DatabaseScript $appOptionFile $restoreDatabase $backupPath
    $restoreTableCount = [int](Get-DatabaseScalar `
        $appOptionFile $restoreDatabase $restoreTableSql)
    if ($restoreTableCount -ne $sourceTableCount) {
        throw 'The restored backup has a different table count.'
    }

    $null = Invoke-EveDbTool 'down'
    $downKdfCount = [int](Get-DatabaseScalar $appOptionFile $database $kdfSql)
    if ($downKdfCount -ne 0) {
        throw 'The password-KDF migration did not roll back.'
    }

    $accountName = "migration_fixture_$PID"
    $migrationFixturePath = Join-Path $workRoot 'migration-fixture.sql'
    Write-Utf8File $migrationFixturePath @"
INSERT INTO account (accountName, password, hash)
VALUES ('$accountName', 'legacy_password_value', 'legacy_hash');
"@
    Invoke-DatabaseScript $appOptionFile $database $migrationFixturePath
    $null = Invoke-EveDbTool 'up'

    $accountSql = "SELECT CONCAT(password, '|', " +
        "IFNULL(passwordKdf, 'NULL'), '|', LENGTH(hash)) " +
        "FROM account WHERE accountName = '$accountName'"
    $accountResult = Get-DatabaseScalar $appOptionFile $database $accountSql
    if ($accountResult -ne '|NULL|11') {
        throw 'The password-KDF migration transformed account data incorrectly.'
    }

    $maxCharacterSql = (
        'SELECT COALESCE(MAX(characterID), 99000000) + 100 FROM chrCharacters'
    )
    $targetCharacterId = [int64](Get-DatabaseScalar `
        $appOptionFile $restoreDatabase $maxCharacterSql)
    $survivorCharacterId = $targetCharacterId + 1
    $maxCorporationSql = (
        'SELECT COALESCE(MAX(corporationID), 99000000) + 100 ' +
        'FROM crpCorporation'
    )
    $corporationId = [int64](Get-DatabaseScalar `
        $appOptionFile $restoreDatabase $maxCorporationSql)
    $maxItemSql = (
        'SELECT COALESCE(MAX(itemID), 140000000) + 100 FROM entity'
    )
    $itemId = [int64](Get-DatabaseScalar $appOptionFile $restoreDatabase $maxItemSql)
    $messageId = $itemId + 1

    $deleteSetupPath = Join-Path $workRoot 'delete-fixture.sql'
    Write-Utf8File $deleteSetupPath @"
SET FOREIGN_KEY_CHECKS = 0;
INSERT INTO crpCorporation
    (corporationID, corporationName, tickerName, url, memberCount,
     creatorID, ceoID)
VALUES
    ($corporationId, 'verification corporation', 'VERIFY', '', 1,
     $targetCharacterId, $targetCharacterId);
INSERT INTO chrCharacters
    (characterID, accountID, characterName, corporationID)
VALUES
    ($targetCharacterId, 0, 'verification target', $corporationId),
    ($survivorCharacterId, 0, 'verification survivor', 0);
INSERT INTO eveMail (messageID, channelID, senderID)
VALUES ($messageId, $targetCharacterId, $targetCharacterId);
INSERT INTO eveMailDetails (messageID, attachment)
VALUES ($messageId, 'fixture');
INSERT INTO webBounties (characterID, ownerID, bounty)
VALUES ($targetCharacterId, $targetCharacterId, 1);
INSERT INTO chrContacts (ownerID, contactID, relationshipID, labelMask)
VALUES ($targetCharacterId, $survivorCharacterId, 1, 0);
INSERT INTO repStandings (fromID, toID, standing)
VALUES ($targetCharacterId, $survivorCharacterId, 1);
INSERT INTO repStandingChanges (fromID, toID, msg)
VALUES ($targetCharacterId, $survivorCharacterId, 'fixture');
INSERT INTO mailStatus (messageID, characterID)
VALUES ($messageId, $targetCharacterId);
INSERT INTO mailMessage (messageID, senderID)
VALUES ($messageId, $targetCharacterId);
INSERT INTO channelChars (charID) VALUES ($targetCharacterId);
INSERT INTO bookmarks (ownerID) VALUES ($targetCharacterId);
INSERT INTO bookmarkFolders (ownerID) VALUES ($targetCharacterId);
INSERT INTO mktOrders (ownerID) VALUES ($targetCharacterId);
INSERT INTO mktTransactions (clientID) VALUES ($targetCharacterId);
INSERT INTO chrCertificates (characterID, certificateID, grantDate)
VALUES ($targetCharacterId, 1, 1);
INSERT INTO chrEmployment (characterID, corporationID, startDate)
VALUES ($targetCharacterId, $corporationId, 1);
INSERT INTO jnlCharacters (ownerID) VALUES ($targetCharacterId);
INSERT INTO crpShares (shareholderID) VALUES ($targetCharacterId);
INSERT INTO chrSkillHistory
    (eventTypeID, characterID, logDate, skillTypeID, skillLevel,
     absolutePoints)
VALUES (1, $targetCharacterId, 1, 1, 1, 1);
INSERT INTO chrSkillQueue (characterID, orderIndex, typeID, level)
VALUES ($targetCharacterId, 1, 1, 1);
INSERT INTO crpApplications
    (corporationID, characterID, applicationText)
VALUES ($corporationId, $targetCharacterId, 'fixture');
INSERT INTO chrCharacterAttributes (charID) VALUES ($targetCharacterId);
INSERT INTO chrPausedSkillQueue
    (characterID, orderIndex, typeID, level)
VALUES ($targetCharacterId, 1, 1, 1);
INSERT INTO chrOwnerNote (ownerID) VALUES ($targetCharacterId);
INSERT INTO chrLabels (ownerID, color, name)
VALUES ($targetCharacterId, 1, 'fixture');
INSERT INTO chrVisitedSystems
    (characterID, solarSystemID, lastDateTime)
VALUES ($targetCharacterId, 1, 1);
INSERT INTO cacheOwners (ownerID) VALUES ($targetCharacterId);
INSERT INTO chrPortraitData (charID) VALUES ($targetCharacterId);
INSERT INTO entity (itemID, ownerID) VALUES ($itemId, $targetCharacterId);
INSERT INTO entity_attributes (itemID, attributeID)
VALUES ($itemId, 1);
INSERT INTO avatar_colors
    (charID, colorID, colorNameA, colorNameBC, weight, gloss)
VALUES ($targetCharacterId, 1, 1, 1, 0, 0);
INSERT INTO avatar_modifiers
    (charID, modifierLocationID, paperdollResourceID)
VALUES ($targetCharacterId, 1, 1);
INSERT INTO avatar_sculpts (charID, sculptLocationID)
VALUES ($targetCharacterId, 1);
INSERT INTO avatars (charID, hairDarkness)
VALUES ($targetCharacterId, 0);
SET FOREIGN_KEY_CHECKS = 1;
"@
    Invoke-DatabaseScript $appOptionFile $restoreDatabase $deleteSetupPath

    $deleteSqlPath = Join-Path $workRoot 'delete-character.sql'
    Write-Utf8File $deleteSqlPath @"
START TRANSACTION;
DELETE FROM eveMailDetails USING eveMail, eveMailDetails
WHERE eveMail.messageID = eveMailDetails.messageID
  AND (senderID = $targetCharacterId OR channelID = $targetCharacterId);
DELETE FROM webBounties
WHERE characterID = $targetCharacterId OR ownerID = $targetCharacterId;
DELETE FROM chrContacts
WHERE ownerID = $targetCharacterId OR contactID = $targetCharacterId;
DELETE FROM repStandings
WHERE fromID = $targetCharacterId OR toID = $targetCharacterId;
DELETE FROM repStandingChanges
WHERE fromID = $targetCharacterId OR toID = $targetCharacterId;
DELETE FROM eveMail
WHERE senderID = $targetCharacterId OR channelID = $targetCharacterId;
DELETE FROM mailStatus WHERE characterID = $targetCharacterId;
DELETE FROM mailMessage WHERE senderID = $targetCharacterId;
DELETE FROM channelChars WHERE charID = $targetCharacterId;
DELETE FROM bookmarks WHERE ownerID = $targetCharacterId;
DELETE FROM bookmarkFolders WHERE ownerID = $targetCharacterId;
DELETE FROM mktOrders WHERE ownerID = $targetCharacterId;
DELETE FROM mktTransactions WHERE clientID = $targetCharacterId;
DELETE FROM chrCertificates WHERE characterID = $targetCharacterId;
DELETE FROM chrEmployment WHERE characterID = $targetCharacterId;
DELETE FROM jnlCharacters WHERE ownerID = $targetCharacterId;
DELETE FROM crpShares WHERE shareholderID = $targetCharacterId;
DELETE FROM chrSkillHistory WHERE characterID = $targetCharacterId;
DELETE FROM chrSkillQueue WHERE characterID = $targetCharacterId;
DELETE FROM crpApplications WHERE characterID = $targetCharacterId;
DELETE FROM chrCharacterAttributes WHERE charID = $targetCharacterId;
DELETE FROM chrPausedSkillQueue WHERE characterID = $targetCharacterId;
DELETE FROM chrOwnerNote WHERE ownerID = $targetCharacterId;
DELETE FROM chrLabels WHERE ownerID = $targetCharacterId;
DELETE FROM chrVisitedSystems WHERE characterID = $targetCharacterId;
DELETE FROM cacheOwners WHERE ownerID = $targetCharacterId;
DELETE FROM chrPortraitData WHERE charID = $targetCharacterId;
DELETE FROM entity_attributes
WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID = $targetCharacterId);
DELETE FROM entity WHERE ownerID = $targetCharacterId;
DELETE FROM avatar_colors WHERE charID = $targetCharacterId;
DELETE FROM avatar_modifiers WHERE charID = $targetCharacterId;
DELETE FROM avatar_sculpts WHERE charID = $targetCharacterId;
DELETE FROM avatars WHERE charID = $targetCharacterId;
DELETE FROM chrCharacters WHERE characterID = $targetCharacterId;
UPDATE crpCorporation
SET memberCount = memberCount - 1
WHERE corporationID = $corporationId AND memberCount > 0;
COMMIT;
"@
    Invoke-DatabaseScript $appOptionFile $restoreDatabase $deleteSqlPath

    $deleteCheckSql = "SELECT CONCAT(" +
        "(SELECT COUNT(*) FROM chrCharacters " +
        "WHERE characterID = $targetCharacterId), '|', " +
        "(SELECT COUNT(*) FROM eveMailDetails " +
        "WHERE messageID = $messageId), '|', " +
        "(SELECT COUNT(*) FROM chrContacts " +
        "WHERE ownerID = $targetCharacterId OR contactID = " +
        "$targetCharacterId), '|', " +
        "(SELECT COUNT(*) FROM entity " +
        "WHERE itemID = $itemId), '|', " +
        "(SELECT COUNT(*) FROM entity_attributes " +
        "WHERE itemID = $itemId), '|', " +
        "(SELECT COUNT(*) FROM crpApplications " +
        "WHERE characterID = $targetCharacterId), '|', " +
        "(SELECT COUNT(*) FROM chrCharacters " +
        "WHERE characterID = $survivorCharacterId), '|', " +
        "(SELECT memberCount FROM crpCorporation " +
        "WHERE corporationID = $corporationId))"
    $deleteResult = Get-DatabaseScalar `
        $appOptionFile $restoreDatabase $deleteCheckSql
    if ($deleteResult -ne '0|0|0|0|0|0|1|0') {
        throw 'Character deletion verification failed.'
    }

    Write-Output (
        "Database verification passed: tables=$sourceTableCount, " +
        "backupBytes=$backupBytes")
}
finally {
    if ($serverProcess -ne $null -and -not $serverProcess.HasExited) {
        try {
            if ($rootOptionFile -ne $null -and
                (Test-Path -LiteralPath $rootOptionFile)) {
                $null = Invoke-Database $rootOptionFile 'mysql' 'SHUTDOWN'
            }
        }
        catch {
            $shutdownError = $_.Exception.Message
        }

        $serverProcess.Refresh()
        if (-not $serverProcess.HasExited) {
            $serverProcess.Kill()
            $serverProcess.WaitForExit(5000) | Out-Null
        }
    }

    foreach ($secretFile in $secretFiles) {
        if (Test-Path -LiteralPath $secretFile) {
            Remove-Item -LiteralPath $secretFile -Force
        }
    }

    if ($workRootCreated -and -not $KeepData) {
        Remove-Item -LiteralPath $workRoot -Recurse -Force
    }
}
