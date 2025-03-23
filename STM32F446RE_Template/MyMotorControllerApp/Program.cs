using System;
using System.IO.Ports;
using System.Threading;

namespace MotorControllerApp
{
    /// <summary>
    /// Entry point for the console application.
    /// Demonstrates a serial-based motor controller using State + Builder patterns.
    /// Includes an incoming data handler that displays device responses.
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            // Instantiate our "context" that holds the current state, the serial port, etc.
            var controllerContext = new MotorControllerContext();

            Console.WriteLine("=== STM32 Motor Controller Console ===");
            Console.WriteLine("Type 'help' for commands.");

            while (true)
            {
                Console.Write("\n> ");
                var input = Console.ReadLine()?.Trim().ToLower();
                if (string.IsNullOrWhiteSpace(input)) continue;

                switch (input)
                {
                    case "help":
                        PrintHelp();
                        break;

                    case "connect":
                        controllerContext.Connect();
                        break;

                    case "disconnect":
                        controllerContext.Disconnect();
                        break;

                    case "estop":
                        // Build "ESTOP" command
                        var eStopCommand = new CommandBuilder()
                            .SetCommandType(CommandType.EmergencyStop)
                            .Build();
                        controllerContext.SendCommand(eStopCommand);
                        break;

                    case var dutyCmd when dutyCmd.StartsWith("set duty "):
                        // e.g. "set duty 50.5"
                        var parts = dutyCmd.Split(' ');
                        if (parts.Length >= 3 && float.TryParse(parts[2], out float dutyValue))
                        {
                            // Build "Dxx.x" command
                            var dutyCommand = new CommandBuilder()
                                .SetCommandType(CommandType.SetDuty)
                                .SetDutyCycle(dutyValue)
                                .Build();

                            controllerContext.SendCommand(dutyCommand);
                        }
                        else
                        {
                            Console.WriteLine("Invalid duty syntax. Try: set duty 50.5");
                        }
                        break;

                    case "exit":
                    case "quit":
                        controllerContext.Disconnect();
                        return;

                    default:
                        Console.WriteLine("Unknown command. Type 'help' for a list.");
                        break;
                }
            }
        }

        private static void PrintHelp()
        {
            Console.WriteLine("Commands:");
            Console.WriteLine("  connect            - Open serial port connection");
            Console.WriteLine("  disconnect         - Close serial port connection");
            Console.WriteLine("  set duty <value>   - e.g. set duty 50.5 (sets 50.5% PWM duty)");
            Console.WriteLine("  estop              - Send emergency stop command");
            Console.WriteLine("  exit, quit         - Exit the application");
        }
    }

    #region Context + State Pattern

    /// <summary>
    /// The "Context" holding our current State (Disconnected/Connected),
    /// plus references to the SerialPort, etc.
    /// </summary>
    public class MotorControllerContext
    {
        private IMotorControllerState _state;

        private string _readBuffer = string.Empty;  // accumulate partial lines


        // Adjust for your actual port name:
        //   - Windows: "COM3"
        //   - macOS: "/dev/tty.usbserial-xxxx"
        //   - Linux: "/dev/ttyUSB0"
        private string _portName = "/dev/tty.usbmodem2303";
        private int    _baudRate = 115200;

        // Our SerialPort object used while connected
        public SerialPort Serial { get; private set; }

        public MotorControllerContext()
        {
            // Start in the DisconnectedState
            _state = new DisconnectedState();
        }

        /// <summary>
        /// Called from Program.Main to attempt connecting.
        /// </summary>
        public void Connect()
        {
            _state.Connect(this);
        }

        /// <summary>
        /// Called from Program.Main to disconnect.
        /// </summary>
        public void Disconnect()
        {
            _state.Disconnect(this);
        }

        /// <summary>
        /// Send a command string over the serial port (if connected).
        /// </summary>
        public void SendCommand(string command)
        {
            _state.SendCommand(this, command);
        }

        /// <summary>
        /// Called by states to finalize the switch from one state to another.
        /// </summary>
        public void ChangeState(IMotorControllerState newState)
        {
            _state = newState;
        }

        /// <summary>
        /// Helper method to open the serial port resource.
        /// Called by DisconnectedState -> Connect().
        /// </summary>
        public void OpenSerialPort()
        {
            try
            {
                Serial = new SerialPort(_portName, _baudRate)
                {
                    Parity     = Parity.None,
                    StopBits   = StopBits.One,
                    DataBits   = 8,
                    Handshake  = Handshake.None,
                    ReadTimeout = 500,
                    WriteTimeout = 500,
                    NewLine = "\r\n" // the typical line terminator for many embedded devices
                };

                // Hook the DataReceived event so we can read incoming data
                Serial.DataReceived += OnDataReceived;

                Serial.Open();
                Console.WriteLine($"[INFO] Serial port {Serial.PortName} opened at {Serial.BaudRate} baud.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ERROR] Could not open serial port: {ex.Message}");
            }
        }

        /// <summary>
        /// Helper method to close the serial port resource.
        /// Called by ConnectedState -> Disconnect().
        /// </summary>
        public void CloseSerialPort()
        {
            if (Serial != null)
            {
                try
                {
                    if (Serial.IsOpen)
                    {
                        Serial.Close();
                    }
                    Serial.DataReceived -= OnDataReceived;
                    Serial.Dispose();
                    Serial = null;
                    Console.WriteLine("[INFO] Serial port closed.");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[ERROR] Closing serial port: {ex.Message}");
                }
            }
        }

        /// <summary>
        /// Event handler for receiving data from the device.
        /// In this simple approach, we read available data
        /// and display it in the console.
        /// </summary>
        private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                var sp = (SerialPort)sender;
                // Read whatever arrived this time
                string incomingChunk = sp.ReadExisting();

                // Accumulate it in our buffer
                _readBuffer += incomingChunk;

                // Process line by line
                string line;
                while ((line = ReadLineFromBuffer(ref _readBuffer)) != null)
                {
                    // 'line' is now a COMPLETE line (without the newline).
                    // Only display or handle lines we actually care about:
                    if (ShouldDisplayLine(line))
                    {
                        Console.WriteLine($"\n[RX] {line}\n> ");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ERROR] OnDataReceived: {ex.Message}");
            }
        }

        /// <summary>
        /// Attempts to extract a single line (delimited by \n) from the buffer.
        /// Returns the line (without the newline) or null if no complete line is found.
        /// 'buffer' is passed by ref so we can modify it.
        /// </summary>
        private string ReadLineFromBuffer(ref string buffer)
        {
            // Look for a newline
            int newLinePos = buffer.IndexOf('\n');
            if (newLinePos >= 0)
            {
                // Extract everything up to this newline
                // Also handle optional \r before \n
                string line = buffer.Substring(0, newLinePos).TrimEnd('\r');
                
                // Remove that portion (plus the \n) from the buffer
                buffer = buffer.Substring(newLinePos + 1);
                
                return line;
            }
            // No newline in the buffer => no complete line yet
            return null;
        }

        /// <summary>
        /// Decide which lines to display vs. ignore.
        /// Return true to display, false to skip.
        /// </summary>
        private bool ShouldDisplayLine(string line)
        {
            // Example: only show lines that contain "Task" or "Duty"
            // You can refine this logic as needed.
            if (line.Contains("Duty") 
            || line.Contains("Stop") 
            || line.Contains("Unknown command"))
            {
                return true;
            }
            // Otherwise, ignore it
            return false;
        }


    }

    /// <summary>
    /// State interface for MotorController
    /// </summary>
    public interface IMotorControllerState
    {
        void Connect(MotorControllerContext context);
        void Disconnect(MotorControllerContext context);
        void SendCommand(MotorControllerContext context, string command);
    }

    /// <summary>
    /// DisconnectedState:
    ///  - Connect() -> tries to open the serial port, transitions to ConnectedState if successful
    ///  - Disconnect() -> no-op
    ///  - SendCommand() -> warns user we're not connected
    /// </summary>
    public class DisconnectedState : IMotorControllerState
    {
        public void Connect(MotorControllerContext context)
        {
            context.OpenSerialPort();
            // If we successfully open, switch state
            if (context.Serial != null && context.Serial.IsOpen)
            {
                context.ChangeState(new ConnectedState());
            }
        }

        public void Disconnect(MotorControllerContext context)
        {
            Console.WriteLine("[WARNING] Already disconnected.");
        }

        public void SendCommand(MotorControllerContext context, string command)
        {
            Console.WriteLine("[WARNING] Can't send command - not connected!");
        }
    }

    /// <summary>
    /// ConnectedState:
    ///  - Connect() -> warns user we're already connected
    ///  - Disconnect() -> closes the serial port, transitions to DisconnectedState
    ///  - SendCommand() -> writes to the serial port
    /// </summary>
    public class ConnectedState : IMotorControllerState
    {
        public void Connect(MotorControllerContext context)
        {
            Console.WriteLine("[WARNING] Already connected.");
        }

        public void Disconnect(MotorControllerContext context)
        {
            context.CloseSerialPort();
            context.ChangeState(new DisconnectedState());
        }

        public void SendCommand(MotorControllerContext context, string command)
        {
            if (context.Serial != null && context.Serial.IsOpen)
            {
                try
                {
                    context.Serial.WriteLine(command); // sends command + NewLine
                    Console.WriteLine($"[TX] {command}");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[ERROR] Failed to send: {ex.Message}");
                }
            }
            else
            {
                Console.WriteLine("[ERROR] Serial port not open?");
            }
        }
    }

    #endregion

    #region Builder Pattern for Commands

    public enum CommandType
    {
        None,
        SetDuty,
        EmergencyStop
    }

    /// <summary>
    /// A builder that helps construct valid motor commands (like "D50.0", "ESTOP").
    /// This makes it easy to expand the protocol later.
    /// </summary>
    public class CommandBuilder
    {
        private CommandType _commandType;
        private float _dutyCycle;

        public CommandBuilder SetCommandType(CommandType commandType)
        {
            _commandType = commandType;
            return this;
        }

        public CommandBuilder SetDutyCycle(float duty)
        {
            _dutyCycle = duty;
            return this;
        }

        public string Build()
        {
            switch (_commandType)
            {
                case CommandType.SetDuty:
                    // e.g., "D50.5"
                    // Optionally clamp to [0..100]
                    if (_dutyCycle < 0f) _dutyCycle = 0f;
                    if (_dutyCycle > 100f) _dutyCycle = 100f;
                    return $"D{_dutyCycle:F1}";

                case CommandType.EmergencyStop:
                    return "ESTOP";

                default:
                    return string.Empty;
            }
        }
    }

    #endregion
}