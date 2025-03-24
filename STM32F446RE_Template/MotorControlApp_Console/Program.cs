using System;
using System.IO.Ports;

namespace MotorControlConsole
{
    //=====================================================================
    // 1) IMotor interface
    //=====================================================================
    public interface IMotor
    {
        void Run();                // from Stopped -> Running
        void Stop();               // from Running -> Stopped
        void EmergencyStop();      // immediate stop
        void SetDuty(float duty);  // 0..100
        void SetSpeed(int rpm);    // (for demo, 1:1 => duty)
    }

    //=====================================================================
    // 2) Abstract Superclass: MotorBase
    //=====================================================================
    public abstract class MotorBase : IMotor
    {
        private string _name;
        protected IMotor inf;

        protected MotorBase(string name)
        {
            _name = name;
        }

        public string Name => _name;

        // Optionally store another IMotor for delegation
        public void SetDelegate(IMotor motor)
        {
            inf = motor;
        }

        // IMotor methods to be overridden
        public abstract void Run();
        public abstract void Stop();
        public abstract void EmergencyStop();
        public abstract void SetDuty(float duty);
        public abstract void SetSpeed(int rpm);
    }

    //=====================================================================
    // 3) State Pattern for the motor: IMotorState, StoppedState, RunningState
    //=====================================================================
    public interface IMotorState
    {
        void Run(BlowerMotor blower);
        void Stop(BlowerMotor blower);
        void EmergencyStop(BlowerMotor blower);
    }

    public class StoppedState : IMotorState
    {
        public void Run(BlowerMotor blower)
        {
            Console.WriteLine("Motor transitions from STOPPED -> RUNNING.");
            blower.SetState(new RunningState());
        }

        public void Stop(BlowerMotor blower)
        {
            Console.WriteLine("Motor is already STOPPED.");
        }

        public void EmergencyStop(BlowerMotor blower)
        {
            Console.WriteLine("Motor is already STOPPED (EmergencyStop does nothing).");
        }
    }

    public class RunningState : IMotorState
    {
        public void Run(BlowerMotor blower)
        {
            Console.WriteLine("Motor is already RUNNING.");
        }

        public void Stop(BlowerMotor blower)
        {
            Console.WriteLine("Motor transitions from RUNNING -> STOPPED.");
            blower.SetState(new StoppedState());
        }

        public void EmergencyStop(BlowerMotor blower)
        {
            Console.WriteLine("Motor is EMERGENCY stopped (RUNNING -> STOPPED).");
            blower.SetState(new StoppedState());
        }
    }

    //=====================================================================
    // 4) Concrete Class: BlowerMotor extends MotorBase
    //    Holds reference to SerialConnectionContext for sending commands
    //=====================================================================
    public class BlowerMotor : MotorBase
    {
        private IMotorState _currentState;
        private SerialConnectionContext _connection;

        public BlowerMotor(string name, SerialConnectionContext connection)
            : base(name)
        {
            _connection = connection;
            _currentState = new StoppedState();
        }

        public override void Run()
        {
            _currentState.Run(this);
        }

        public override void Stop()
        {
            _currentState.Stop(this);

            var cmd = new MotorCommandBuilder()
                .SetCommandType(CommandType.SetDuty)
                .SetValue(0.0f)
                .Build();

            Console.WriteLine($"[Motor] Stopping => {cmd}");
            _connection.SendCommand(cmd);
        }

        public override void EmergencyStop()
        {
            _currentState.EmergencyStop(this);

            var cmd = new MotorCommandBuilder()
                .SetCommandType(CommandType.EmergencyStop)
                .Build();

            Console.WriteLine($"[Motor] Emergency Stop => {cmd}");
            _connection.SendCommand(cmd);
        }

        public void SetState(IMotorState newState)
        {
            _currentState = newState;
        }

        public override void SetDuty(float duty)
        {

            var cmd = new MotorCommandBuilder()
                .SetCommandType(CommandType.SetDuty)
                .SetValue(duty)
                .Build();

            Console.WriteLine($"[Motor] Setting duty = {duty:F1}% => {cmd}");
            _connection.SendCommand(cmd);
        }

        public override void SetSpeed(int rpm)
        {
            // Simple 1:1 ratio => duty = rpm
            float dutyEquivalent = rpm;
            if (dutyEquivalent < 0) dutyEquivalent = 0;
            if (dutyEquivalent > 100) dutyEquivalent = 100;

            // We'll reuse the SetDuty command
            var cmd = new MotorCommandBuilder()
                .SetCommandType(CommandType.SetDuty) 
                .SetValue(dutyEquivalent)
                .Build();

            Console.WriteLine($"[Motor] Setting speed = {rpm} => duty {dutyEquivalent:F1}% => {cmd}");
            _connection.SendCommand(cmd);
        }
    }

    //=====================================================================
    // 5) Builder Pattern: MotorCommandBuilder
    //=====================================================================
    public enum CommandType
    {
        None,
        SetDuty,
        SetRpm,         // not used here, but left for expansion
        EmergencyStop
    }

    public class MotorCommandBuilder
    {
        private CommandType _cmdType;
        private float _value;

        public MotorCommandBuilder SetCommandType(CommandType cmdType)
        {
            _cmdType = cmdType;
            return this;
        }

        public MotorCommandBuilder SetValue(float value)
        {
            _value = value;
            return this;
        }

        public string Build()
        {
            switch (_cmdType)
            {
                case CommandType.SetDuty:
                    return $"D{_value:F1}";
                case CommandType.SetRpm:
                    return $"R{(int)_value}";
                case CommandType.EmergencyStop:
                    return "ESTOP";
                default:
                    return string.Empty;
            }
        }
    }

    //=====================================================================
    // 6) Connection Management with State Pattern
    //=====================================================================
    public interface IConnectionState
    {
        void Connect(SerialConnectionContext ctx, string portName);
        void Disconnect(SerialConnectionContext ctx);
        void SendCommand(SerialConnectionContext ctx, string command);
    }

    public class DisconnectedState : IConnectionState
    {
        public void Connect(SerialConnectionContext ctx, string portName)
        {
            ctx.OpenPort(portName);
            if (ctx.Port != null && ctx.Port.IsOpen)
            {
                ctx.ChangeState(new ConnectedState());
            }
        }

        public void Disconnect(SerialConnectionContext ctx)
        {
            Console.WriteLine("[WARNING] Already disconnected.");
        }

        public void SendCommand(SerialConnectionContext ctx, string command)
        {
            Console.WriteLine("[WARNING] Can't send command - not connected!");
        }
    }

    public class ConnectedState : IConnectionState
    {
        public void Connect(SerialConnectionContext ctx, string portName)
        {
            Console.WriteLine("[WARNING] Already connected.");
        }

        public void Disconnect(SerialConnectionContext ctx)
        {
            ctx.ClosePort();
            ctx.ChangeState(new DisconnectedState());
        }

        public void SendCommand(SerialConnectionContext ctx, string command)
        {
            if (ctx.Port != null && ctx.Port.IsOpen)
            {
                ctx.Port.WriteLine(command);
                Console.WriteLine($"[TX] {command}");
            }
            else
            {
                Console.WriteLine("[ERROR] Port is not open??");
            }
        }
    }

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

    //=====================================================================
    // 7) Main Program: Console Menu
    //=====================================================================
    class Program
    {
        static void Main()
        {
            // 1) Create our connection context (manages COM port states)
            var connection = new SerialConnectionContext();

            // 2) Create the blower motor, giving it a reference to the connection
            BlowerMotor blower = new BlowerMotor("Koford CPAP Blower", connection);
            Console.WriteLine($"Created: {blower.Name}\n");

            bool done = false;
            while (!done)
            {
                Console.WriteLine("Commands:");
                Console.WriteLine("  listports          - Show available COM ports");
                Console.WriteLine("  connect <port>     - e.g. connect COM3");
                Console.WriteLine("  disconnect         - Close COM port");
                Console.WriteLine("  run                - Motor from STOPPED -> RUNNING");
                Console.WriteLine("  stop               - Motor STOP");
                Console.WriteLine("  estop              - Motor emergency stop (ESTOP)");
                Console.WriteLine("  setduty <float>    - e.g. setduty 50.5 => 'D50.5'");
                Console.WriteLine("  setspeed <int>     - e.g. setspeed 30 => 'D30.0' if 1:1 ratio");
                Console.WriteLine("  quit               - Exit");
                Console.Write("> ");
                var input = Console.ReadLine()?.Trim();
                if (string.IsNullOrEmpty(input)) continue;

                var parts = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                var cmd = parts[0].ToLower();

                switch (cmd)
                {
                    case "quit":
                        done = true;
                        break;

                    case "listports":
                        SerialConnectionContext.ListAvailablePorts();
                        break;

                    case "connect":
                        if (parts.Length < 2)
                        {
                            Console.WriteLine("Usage: connect <portName>");
                            break;
                        }
                        connection.Connect(parts[1]);
                        break;

                    case "disconnect":
                        connection.Disconnect();
                        break;

                    case "run":
                        blower.Run();
                        break;

                    case "stop":
                        blower.Stop();
                        break;

                    case "estop":
                        blower.EmergencyStop();
                        break;

                    case "setduty":
                        if (parts.Length < 2)
                        {
                            Console.WriteLine("Usage: setduty <float>");
                            break;
                        }
                        if (float.TryParse(parts[1], out float dutyVal))
                        {
                            blower.SetDuty(dutyVal);
                        }
                        else
                        {
                            Console.WriteLine("Invalid duty value.");
                        }
                        break;

                    case "setspeed":
                        if (parts.Length < 2)
                        {
                            Console.WriteLine("Usage: setspeed <int>");
                            break;
                        }
                        if (int.TryParse(parts[1], out int rpmVal))
                        {
                            blower.SetSpeed(rpmVal);
                        }
                        else
                        {
                            Console.WriteLine("Invalid speed value.");
                        }
                        break;

                    default:
                        Console.WriteLine("Unknown command.");
                        break;
                }

                Console.WriteLine();
            }

            // Cleanly disconnect if open
            connection.Disconnect();
            Console.WriteLine("Exiting program.");
        }
    }
}
