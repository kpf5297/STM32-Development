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
}
