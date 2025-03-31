using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
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
}
