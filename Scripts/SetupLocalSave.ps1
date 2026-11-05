$ErrorActionPreference = 'Stop'
function Invoke-LocalSql([string]$Database, [string]$Query) {
    $connection = [System.Data.SqlClient.SqlConnection]::new(
        "Server=lpc:.\SQLEXPRESS;Database=$Database;Integrated Security=True;Encrypt=True;TrustServerCertificate=True;Connect Timeout=5")
    try {
        $connection.Open()
        $command = $connection.CreateCommand()
        $command.CommandText = $Query
        $command.CommandTimeout = 15
        [void]$command.ExecuteNonQuery()
    } finally {
        $connection.Dispose()
    }
}
Invoke-LocalSql master @"
IF DB_ID(N'MyFarmEngineLocal') IS NULL
    CREATE DATABASE MyFarmEngineLocal;
"@
Invoke-LocalSql MyFarmEngineLocal @"
SET XACT_ABORT ON;
BEGIN TRANSACTION;
IF OBJECT_ID(N'dbo.FarmSave', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.FarmSave (
        slot int NOT NULL PRIMARY KEY CHECK (slot IN (1,2)),
        payload varchar(max) NULL CHECK (DATALENGTH(payload) BETWEEN 8 AND 32768),
        previous_payload varchar(max) NULL CHECK (DATALENGTH(previous_payload) BETWEEN 8 AND 32768)
    );
END;
IF NOT EXISTS (SELECT 1 FROM dbo.FarmSave WITH (UPDLOCK,HOLDLOCK) WHERE slot=1)
    INSERT dbo.FarmSave(slot) VALUES (1);
IF NOT EXISTS (SELECT 1 FROM dbo.FarmSave WITH (UPDLOCK,HOLDLOCK) WHERE slot=2)
    INSERT dbo.FarmSave(slot) VALUES (2);
COMMIT;
"@
Write-Output 'MyFarmEngineLocal ready. Slot 1 is gameplay and slot 2 is integration testing.'
