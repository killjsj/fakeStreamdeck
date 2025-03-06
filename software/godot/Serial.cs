using Godot;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

public partial class Serial : Node2D
{
    private SerialPort _port;
    private readonly List<PortStatus> _failedPorts = new();
    private const string EOF_MARKER = "Eof";
    private CancellationTokenSource _cts;

    [Signal]
    public delegate void data_receivedEventHandler(string data);

    private class PortStatus
    {
        public string PortName { get; set; }
        public int FailCount { get; set; }

        public override bool Equals(object obj) =>
            obj is PortStatus other && PortName == other.PortName;

        public override int GetHashCode() => PortName.GetHashCode();
    }

    public override void _Ready()
    {
        _cts = new CancellationTokenSource();
        _ = ProcessSerialPortsAsync(_cts.Token);
    }

    public override void _ExitTree()
    {
        base._ExitTree();
        _cts?.Cancel();
        _port?.Dispose();
    }

    

    private async Task ScanAndConnectPortAsync(CancellationToken ct)
    {
        foreach (string portName in SerialPort.GetPortNames())
        {
            if (IsPortBlacklisted(portName) || ct.IsCancellationRequested)
                continue;

            try
            {
                if (await TryConnectPortAsync(portName, ct))
                {
                    GD.Print($"Successfully connected to {portName}");
                    return;
                }
            }
            catch (Exception ex)
            {
                GD.PrintErr($"Failed to connect to {portName}: {ex.Message}");
                AddToBlacklist(portName);
            }
        }
    }
    void DataReceivedHandler(object s, SerialDataReceivedEventArgs e)
    {
        try
        {
            if (port?.IsOpen == true)
            {
                string data = port.ReadTo(EOF_MARKER);
                if (!string.IsNullOrEmpty(data))
                {
                    dataReceived.TrySetResult(data);
                }
            }
        }
        catch (Exception ex) when (ex is TimeoutException || ex is InvalidOperationException)
        {
            dataReceived.TrySetException(ex);
        }
    }
        SerialPort port = null;
            TaskCompletionSource<string> dataReceived = new TaskCompletionSource<string>();

    private const int CONNECTION_TIMEOUT = 3000; // 连接超时时间(毫秒)
    private const int READ_WRITE_TIMEOUT = 1000; // 读写超时时间(毫秒)
       private async Task ProcessSerialPortsAsync(CancellationToken ct)
    {
        while (!ct.IsCancellationRequested)
        {
            try
            {
                // 如果已经有连接的端口，只需要确认连接状态
                if (_port?.IsOpen == true)
                {
                    await Task.Delay(1000, ct);
                    continue;
                }

                await ScanAndConnectPortAsync(ct);
                await Task.Delay(1000, ct);
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (Exception ex)
            {
                GD.PrintErr($"Serial processing error: {ex.Message}");
                // 如果是已连接端口出错，清除当前连接
                if (_port != null)
                {
                    _port.Dispose();
                    _port = null;
                }
                await Task.Delay(2000, ct);
            }
        }
    }

    private async Task<bool> TryConnectPortAsync(string portName, CancellationToken ct)
    {
        SerialPort tempPort = null;
        var dataReceived = new TaskCompletionSource<string>();

        try
        {
            tempPort = new SerialPort(portName, 115200, Parity.None, 8, StopBits.One)
            {
                Encoding = Encoding.UTF8,
                ReadTimeout = READ_WRITE_TIMEOUT,
                WriteTimeout = READ_WRITE_TIMEOUT
            };

            void DataReceivedHandler(object s, SerialDataReceivedEventArgs e)
            {
                try
                {
                    if (tempPort?.IsOpen == true)
                    {
                        string data = tempPort.ReadTo(EOF_MARKER);
                        if (!string.IsNullOrEmpty(data))
                        {
                            dataReceived.TrySetResult(data);
                        }
                    }
                }
                catch (Exception ex) when (ex is TimeoutException || ex is InvalidOperationException)
                {
                    dataReceived.TrySetException(ex);
                }
            }

            tempPort.DataReceived += DataReceivedHandler;

            using var timeoutCts = new CancellationTokenSource(CONNECTION_TIMEOUT);
            using var linkedCts = CancellationTokenSource.CreateLinkedTokenSource(ct, timeoutCts.Token);

            await Task.Run(() => tempPort.Open(), linkedCts.Token);

            try
            {
                string data = await dataReceived.Task.WaitAsync(linkedCts.Token);

                if (!string.IsNullOrEmpty(data))
                {
                    _port = tempPort;
                    CallDeferred(nameof(EmitSignal), "data_received", data);
                    GD.Print($"Connected to {portName} at 115200 baud");
                    return true;
                }
            }
            catch (OperationCanceledException)
            {
                throw new TimeoutException($"Connection timeout on {portName}");
            }
        }
        catch
        {
            if (tempPort != null)
            {
                tempPort.DataReceived -= DataReceivedHandler;
                if (tempPort.IsOpen)
                {
                    tempPort.Close();
                }
                tempPort.Dispose();
            }
            throw;
        }
        return false;
    }

    private bool IsPortBlacklisted(string portName)
    {
        var port = _failedPorts.Find(p => p.PortName == portName);
        return port?.FailCount >= 2;
    }

    private void AddToBlacklist(string portName)
    {
        var port = _failedPorts.Find(p => p.PortName == portName);
        if (port == null)
        {
            _failedPorts.Add(new PortStatus { PortName = portName, FailCount = 1 });
        }
        else
        {
            port.FailCount++;
        }
    }
}