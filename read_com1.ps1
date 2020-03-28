$PipeName = 'metalos_com1'
$PipeDir  = [System.IO.Pipes.PipeDirection]::In
$PipeOpt  = [System.IO.Pipes.PipeOptions]::Asynchronous

try
{
    $pipeClient = new-object System.IO.Pipes.NamedPipeClientStream(".", $PipeName, $PipeDir, $PipeOpt)
    $reader = new-object System.IO.StreamReader($pipeClient)
    $pipeClient.Connect()
    if (!$pipeClient.IsConnected)
    {
        throw "Failed to connect client to pipe $pipeName"
    }

    while (!$reader.EndOfStream)
    {
        Write-Host $reader.ReadLine()
    }
}
catch
{
    Write-Host "Error reading from pipe: $_" -ForegroundColor Red
}
finally
{
    if ($reader)
    {
        $reader.Dispose()
        $reader = $null
    }
    if ($pipeClient)
    {
        $pipeClient.Dispose()
        $pipeClient = $null
    }
}