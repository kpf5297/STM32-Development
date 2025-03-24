using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public class SerialConnectionContext
    {
        private IConnectionState _state;
        public SerialPort Port { get; private set; }

        public SerialConnectionContext()
        {
            _state = new DisconnectedState();
        }

        public void Connect(string portName)
        {
            _state.Connect(this, portName);
        }

        public void Disconnect()
        {
            _state.Disconnect(this);
        }

        public void SendCommand(string cmd)
        {
            _state.SendCommand(this, cmd);
        }

        public void ChangeState(IConnectionState newState)
        {
            _state = newState;
        }

        // Actually open the port
        public void OpenPort(string portName)
        {
            try
            {
                Port = new SerialPort(portName, 115200)
                {
                    Parity = Parity.None,
                    StopBits = StopBits.One,
                    DataBits = 8,
                    Handshake = Handshake.None,
                    ReadTimeout = 500,
                    WriteTimeout = 500
                };
                Port.Open();
                Console.WriteLine($"[INFO] Connected on {Port.PortName} at {Port.BaudRate} baud.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ERROR] Opening port failed: {ex.Message}");
            }
        }

        // Actually close the port
        public void ClosePort()
        {
            if (Port != null && Port.IsOpen)
            {
                Port.Close();
                Port.Dispose();
                Port = null;
                Console.WriteLine("[INFO] Port closed.");
            }
        }

        // Utility for listing ports
        public static void ListAvailablePorts()
        {
            var ports = SerialPort.GetPortNames();
            Console.WriteLine("Available ports:");
            if (ports.Length == 0)
            {
                Console.WriteLine("  (none found)");
            }
            else
            {
                foreach (var p in ports) Console.WriteLine("  " + p);
            }
        }
    }
}
