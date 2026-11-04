param([Parameter(Mandatory=$true)][string]$Game,
      [Parameter(Mandatory=$true)][int]$ExpectedSubsystem)
$ErrorActionPreference='Stop'
$bytes=[IO.File]::ReadAllBytes($Game)
$pe=[BitConverter]::ToInt32($bytes,0x3c)
$subsystem=[BitConverter]::ToUInt16($bytes,$pe+24+68)
if($subsystem -ne $ExpectedSubsystem) { throw "PE subsystem $subsystem, expected $ExpectedSubsystem" }
$logRoot=Join-Path (Split-Path $Game -Parent) ('launch-test-'+[Guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($logRoot) | Out-Null
$oldLog=$env:MYFARM_LOG_DIR
$env:MYFARM_LOG_DIR=$logRoot
try {
    foreach($testCase in @(@{Args='--smoke-test';Code=0;Text='Normal shutdown'},
                          @{Args='--unknown-gameplay11-option';Code=2;Text='Usage:'})) {
        $tag=if($testCase.Code -eq 0){'smoke'}else{'invalid'}
        $out=Join-Path $logRoot ($tag+'.stdout.txt')
        $err=Join-Path $logRoot ($tag+'.stderr.txt')
        $process=Start-Process -FilePath $Game -ArgumentList $testCase.Args -WorkingDirectory (Split-Path $Game -Parent) -WindowStyle Hidden -RedirectStandardOutput $out -RedirectStandardError $err -PassThru
        if(!$process.WaitForExit(30000)) { $process.Kill();throw 'Launch test timeout' }
        $process.Refresh()
        if($process.ExitCode -ne $testCase.Code) { throw "Unexpected exit code $($process.ExitCode)" }
        $log=Get-Content -LiteralPath (Join-Path $logRoot ('MyFarmEngine-'+$process.Id+'.log')) -Raw
        if(!$log.Contains($testCase.Text)) { throw 'Persistent log lost diagnostics' }
        $captured=(Get-Content -LiteralPath $out -Raw)+(Get-Content -LiteralPath $err -Raw)
        if(!$captured.Contains($testCase.Text)) { throw 'Redirected stdout/stderr lost diagnostics' }
        if($ExpectedSubsystem -eq 2 -and !$log.Contains('Console attached: no')) { throw 'GUI launch acquired a console' }
        Write-Output "PASS: subsystem=$subsystem, args=$($testCase.Args), exit=$($testCase.Code), file and redirected logs present"
    }
} finally { $env:MYFARM_LOG_DIR=$oldLog }
