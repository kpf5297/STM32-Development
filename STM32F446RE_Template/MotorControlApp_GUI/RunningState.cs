using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
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

}
