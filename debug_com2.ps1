$PipeName = 'metalos_com2'
$PipeDir  = [System.IO.Pipes.PipeDirection]::Out
$PipeOpt  = [System.IO.Pipes.PipeOptions]::Asynchronous

try
{
    $pipeClient = new-object System.IO.Pipes.NamedPipeClientStream(".", $PipeName, $PipeDir, $PipeOpt)

    $writer = new-object System.IO.StreamWriter($pipeClient)
    $pipeClient.Connect()
    if (!$pipeClient.IsConnected)
    {
        throw "Failed to connect client to pipe $pipeName"
    }

    $writer.WriteLine("ThisIsATestOfUartDebugging\n");
    $writer.Flush();

    Write-Host "done"
}
catch
{
    Write-Host "Error reading from pipe: $_" -ForegroundColor Red
}
finally
{
    if ($writer)
    {
        $writer.Dispose()
        $writer = $null
    }
    if ($pipeClient)
    {
        $pipeClient.Dispose()
        $pipeClient = $null
    }
}