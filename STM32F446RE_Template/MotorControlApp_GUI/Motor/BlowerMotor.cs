using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
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
            rpm = Math.Max(0, Math.Min(100000, rpm));
            dutyEquivalent = Math.Max(0, Math.Min(100.0f, dutyEquivalent));

            dutyEquivalent = (float)Math.Round(dutyEquivalent, 1);
            if (dutyEquivalent == 0.0f)
            {
                dutyEquivalent = 0.0f;
            }
            else if (dutyEquivalent < 0.0f)
            {
                dutyEquivalent = 0.0f;
            }
            else if (dutyEquivalent > 100.0f)
            {
                dutyEquivalent = 100.0f;
            }

            var cmd = new MotorCommandBuilder()
                .SetCommandType(CommandType.SetDuty)
                .SetValue(dutyEquivalent)
                .Build();

            Console.WriteLine($"[Motor] Setting speed = {rpm} => duty {dutyEquivalent:F1}% => {cmd}");
            _connection.SendCommand(cmd);
        }
    }
}
